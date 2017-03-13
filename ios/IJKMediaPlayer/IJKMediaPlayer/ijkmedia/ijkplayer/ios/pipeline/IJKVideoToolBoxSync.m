/*****************************************************************************
 * IJKVideoToolBox.m
 *****************************************************************************
 *
 * copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
 *
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ijkPlayer; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "IJKVideoToolBoxSync.h"
#include "ijksdl_vout_overlay_videotoolbox.h"
#include "ffpipeline_ios.h"
#include <mach/mach_time.h>
#include "libavformat/avc.h"
#include "ijksdl_vout_ios_gles2.h"
#include "h264_sps_parser.h"
#include "ijkplayer/ff_ffplay_debug.h"
#import <CoreMedia/CoreMedia.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreVideo/CVHostTime.h>
#import <Foundation/Foundation.h>
#import "IJKDeviceModel.h"
#include <stdatomic.h>
#import <VideoToolbox/VideoToolbox.h>
#include "ff_ffinc.h"
#include "ff_fferror.h"
#include "ff_ffmsg.h"
#include "ijksdl/ios/ijksdl_vout_overlay_videotoolbox.h"

#define IJK_VTB_FCC_AVCC   SDL_FOURCC('C', 'c', 'v', 'a')

#define MAX_PKT_QUEUE_DEEP   350

typedef struct sample_info {

    double  sort;
    double  dts;
    double  pts;
    int     serial;

    int     sar_num;
    int     sar_den;

} sample_info;

typedef struct sort_queue {
    AVFrame pic;
    int serial;
    int64_t sort;
    volatile struct sort_queue *nextframe;
} sort_queue;

typedef struct VTBFormatDesc
{
    CMFormatDescriptionRef      fmt_desc;
    int32_t                     max_ref_frames;
    bool                        convert_bytestream;
    bool                        convert_3byteTo4byteNALSize;
} VTBFormatDesc;

struct Ijk_VideoToolBox_Opaque {
    FFPlayer                   *ffp;
    VTDecompressionSessionRef   vt_session;

    AVCodecParameters          *codecpar;
    VTBFormatDesc               fmt_desc;


    volatile bool               refresh_request;
    volatile bool               new_seg_flag;
    volatile bool               idr_based_identified;
    volatile bool               refresh_session;
    volatile bool               recovery_drop_packet;

    pthread_mutex_t             m_queue_mutex;
    volatile sort_queue        *m_sort_queue;
    volatile int32_t            m_queue_depth;

    int                         m_buffer_deep;
    AVPacket                    m_buffer_packet[MAX_PKT_QUEUE_DEEP];

    sample_info                 sample_info;

    SDL_SpeedSampler            sampler;

    int                         serial;
    bool                        dealloced;

};

static void vtbformat_destroy(VTBFormatDesc *fmt_desc);
static int  vtbformat_init(VTBFormatDesc *fmt_desc, AVCodecParameters *codecpar);

static const char *vtb_get_error_string(OSStatus status) {
    switch (status) {
        case kVTInvalidSessionErr:                      return "kVTInvalidSessionErr";
        case kVTVideoDecoderBadDataErr:                 return "kVTVideoDecoderBadDataErr";
        case kVTVideoDecoderUnsupportedDataFormatErr:   return "kVTVideoDecoderUnsupportedDataFormatErr";
        case kVTVideoDecoderMalfunctionErr:             return "kVTVideoDecoderMalfunctionErr";
        default:                                        return "UNKNOWN";
    }
}

static void SortQueuePop(Ijk_VideoToolBox_Opaque* context)
{
    if (!context->m_sort_queue || context->m_queue_depth == 0) {
        return;
    }
    pthread_mutex_lock(&context->m_queue_mutex);
    volatile sort_queue *top_frame = context->m_sort_queue;
    context->m_sort_queue = context->m_sort_queue->nextframe;
    context->m_queue_depth--;
    pthread_mutex_unlock(&context->m_queue_mutex);
    CVBufferRelease(top_frame->pic.opaque);
    free((void*)top_frame);
}

static void SortQueuePush(Ijk_VideoToolBox_Opaque *ctx, sort_queue *newFrame)
{
    pthread_mutex_lock(&ctx->m_queue_mutex);
    volatile sort_queue *queueWalker = ctx->m_sort_queue;
    if (!queueWalker || (newFrame->sort < queueWalker->sort)) {
        newFrame->nextframe = queueWalker;
        ctx->m_sort_queue = newFrame;
    } else {
        bool frameInserted = false;
        volatile sort_queue *nextFrame = NULL;
        while (!frameInserted) {
            nextFrame = queueWalker->nextframe;
            if (!nextFrame || (newFrame->sort < nextFrame->sort)) {
                newFrame->nextframe = nextFrame;
                queueWalker->nextframe = newFrame;
                frameInserted = true;
            }
            queueWalker = nextFrame;
        }
    }
    ctx->m_queue_depth++;
    pthread_mutex_unlock(&ctx->m_queue_mutex);
}

static void CFDictionarySetSInt32(CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32)
{
    CFNumberRef number;
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &numberSInt32);
    CFDictionarySetValue(dictionary, key, number);
    CFRelease(number);
}

static void CFDictionarySetBoolean(CFMutableDictionaryRef dictionary, CFStringRef key, BOOL value)
{
    CFDictionarySetValue(dictionary, key, value ? kCFBooleanTrue : kCFBooleanFalse);
}

static CMSampleBufferRef CreateSampleBufferFrom(CMFormatDescriptionRef fmt_desc, void *demux_buff, size_t demux_size)
{
    OSStatus status;
    CMBlockBufferRef newBBufOut = NULL;
    CMSampleBufferRef sBufOut = NULL;

    status = CMBlockBufferCreateWithMemoryBlock(
                                                NULL,
                                                demux_buff,
                                                demux_size,
                                                kCFAllocatorNull,
                                                NULL,
                                                0,
                                                demux_size,
                                                FALSE,
                                                &newBBufOut);

    if (!status) {
        status = CMSampleBufferCreate(
                                      NULL,
                                      newBBufOut,
                                      TRUE,
                                      0,
                                      0,
                                      fmt_desc,
                                      1,
                                      0,
                                      NULL,
                                      0,
                                      NULL,
                                      &sBufOut);
    }

    CFRelease(newBBufOut);
    if (status == 0) {
        return sBufOut;
    } else {
        return NULL;
    }
}

static bool GetVTBPicture(Ijk_VideoToolBox_Opaque* context, AVFrame* pVTBPicture)
{
    if (context->m_sort_queue == NULL) {
        return false;
    }
    pthread_mutex_lock(&context->m_queue_mutex);

    volatile sort_queue *sort_queue = context->m_sort_queue;
    *pVTBPicture        = sort_queue->pic;
    pVTBPicture->opaque = CVBufferRetain(sort_queue->pic.opaque);

    pthread_mutex_unlock(&context->m_queue_mutex);

    return true;
}

static void QueuePicture(Ijk_VideoToolBox_Opaque* ctx) {
    AVFrame picture = {0};
    if (true == GetVTBPicture(ctx, &picture)) {
        AVRational tb = ctx->ffp->is->video_st->time_base;
        AVRational frame_rate = av_guess_frame_rate(ctx->ffp->is->ic, ctx->ffp->is->video_st, NULL);
        double duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational) {frame_rate.den, frame_rate.num}) : 0);
        double pts = (picture.pts == AV_NOPTS_VALUE) ? NAN : picture.pts * av_q2d(tb);

        picture.format = IJK_AV_PIX_FMT__VIDEO_TOOLBOX;

        ffp_queue_picture(ctx->ffp, &picture, pts, duration, 0, ctx->ffp->is->viddec.pkt_serial);

        CVBufferRelease(picture.opaque);

        SortQueuePop(ctx);
    } else {
        ALOGI("Get Picture failure!!!\n");
    }
}


static void VTDecoderCallback(void *decompressionOutputRefCon,
                       void *sourceFrameRefCon,
                       OSStatus status,
                       VTDecodeInfoFlags infoFlags,
                       CVImageBufferRef imageBuffer,
                       CMTime presentationTimeStamp,
                       CMTime presentationDuration)
{
    @autoreleasepool {
        Ijk_VideoToolBox_Opaque *ctx = (Ijk_VideoToolBox_Opaque*)decompressionOutputRefCon;
        if (!ctx)
            return;

        FFPlayer   *ffp         = ctx->ffp;
        VideoState *is          = ffp->is;
        sort_queue *newFrame    = NULL;

        sample_info *sample_info = &ctx->sample_info;

        newFrame = (sort_queue *)mallocz(sizeof(sort_queue));
        if (!newFrame) {
            ALOGE("VTB: create new frame fail: out of memory\n");
            goto failed;
        }

        newFrame->pic.pts        = sample_info->pts;
        newFrame->pic.pkt_dts    = sample_info->dts;
        newFrame->pic.sample_aspect_ratio.num = sample_info->sar_num;
        newFrame->pic.sample_aspect_ratio.den = sample_info->sar_den;
        newFrame->serial     = sample_info->serial;
        newFrame->nextframe  = NULL;

        if (newFrame->pic.pts != AV_NOPTS_VALUE) {
            newFrame->sort    = newFrame->pic.pts;
        } else {
            newFrame->sort    = newFrame->pic.pkt_dts;
            newFrame->pic.pts = newFrame->pic.pkt_dts;
        }

        if (ctx->dealloced || is->abort_request || is->viddec.queue->abort_request)
            goto failed;

        if (status != 0) {
            ALOGE("decode callback %d %s\n", (int)status, vtb_get_error_string(status));
            goto failed;
        }

        if (ctx->refresh_session) {
            goto failed;
        }

        if (newFrame->serial != ctx->serial) {
            goto failed;
        }

        if (imageBuffer == NULL) {
            ALOGI("imageBuffer null\n");
            goto failed;
        }

        ffp->stat.vdps = SDL_SpeedSamplerAdd(&ctx->sampler, FFP_SHOW_VDPS_VIDEOTOOLBOX, "vdps[VideoToolbox]");
#ifdef FFP_VTB_DISABLE_OUTPUT
        goto failed;
#endif

        OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
        if (format_type != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            ALOGI("format_type error \n");
            goto failed;
        }
        if (kVTDecodeInfo_FrameDropped & infoFlags) {
            ALOGI("droped\n");
            goto failed;
        }

        if (ctx->new_seg_flag) {
            ALOGI("new seg process!!!!");
            while (ctx->m_queue_depth > 0) {
                QueuePicture(ctx);
            }
            ctx->new_seg_flag = false;
        }

        if (ctx->m_sort_queue && newFrame->pic.pts < ctx->m_sort_queue->pic.pts) {
            goto failed;
        }

        // FIXME: duplicated code
        {
            double dpts = NAN;

            if (newFrame->pic.pts != AV_NOPTS_VALUE)
                dpts = av_q2d(is->video_st->time_base) * newFrame->pic.pts;

            if (ffp->framedrop>0 || (ffp->framedrop && ffp_get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
                if (newFrame->pic.pts != AV_NOPTS_VALUE) {
                    double diff = dpts - ffp_get_master_clock(is);
                    if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                        diff - is->frame_last_filter_delay < 0 &&
                        is->viddec.pkt_serial == is->vidclk.serial &&
                        is->videoq.nb_packets) {
                        is->frame_drops_early++;
                        is->continuous_frame_drops_early++;
                        if (is->continuous_frame_drops_early > ffp->framedrop) {
                            is->continuous_frame_drops_early = 0;
                        } else {
                            // drop too late frame
                            goto failed;
                        }
                    }
                }
            }
        }

        if (CVPixelBufferIsPlanar(imageBuffer)) {
            newFrame->pic.width  = (int)CVPixelBufferGetWidthOfPlane(imageBuffer, 0);
            newFrame->pic.height = (int)CVPixelBufferGetHeightOfPlane(imageBuffer, 0);
        } else {
            newFrame->pic.width  = (int)CVPixelBufferGetWidth(imageBuffer);
            newFrame->pic.height = (int)CVPixelBufferGetHeight(imageBuffer);
        }


        newFrame->pic.opaque = CVBufferRetain(imageBuffer);
        SortQueuePush(ctx, newFrame);

        if (ctx->ffp->is == NULL || ctx->ffp->is->abort_request || ctx->ffp->is->viddec.queue->abort_request) {
            while (ctx->m_queue_depth > 0) {
                SortQueuePop(ctx);
            }
            goto successed;
        }

        if ((ctx->m_queue_depth > ctx->fmt_desc.max_ref_frames)) {
            QueuePicture(ctx);
        }
    successed:
        return;
    failed:
        if (newFrame) {
            free(newFrame);
        }
        return;
    }
}

static void vtbsession_destroy(Ijk_VideoToolBox_Opaque *context)
{
    if (!context)
        return;

    vtbformat_destroy(&context->fmt_desc);

    if (context->vt_session) {
        VTDecompressionSessionWaitForAsynchronousFrames(context->vt_session);
        VTDecompressionSessionInvalidate(context->vt_session);
        CFRelease(context->vt_session);
        context->vt_session = NULL;
    }
}

static VTDecompressionSessionRef vtbsession_create(Ijk_VideoToolBox_Opaque* context)
{
    FFPlayer *ffp = context->ffp;
    int       ret = 0;
    int       width  = context->codecpar->width;
    int       height = context->codecpar->height;

    VTDecompressionSessionRef vt_session = NULL;
    CFMutableDictionaryRef destinationPixelBufferAttributes;
    VTDecompressionOutputCallbackRecord outputCallback;
    OSStatus status;

    ret = vtbformat_init(&context->fmt_desc, context->codecpar);

    if (ffp->vtb_max_frame_width > 0 && width > ffp->vtb_max_frame_width) {
        double w_scaler = (float)ffp->vtb_max_frame_width / width;
        width = ffp->vtb_max_frame_width;
        height = height * w_scaler;
    }

    ALOGI("after scale width %d height %d \n", width, height);

    destinationPixelBufferAttributes = CFDictionaryCreateMutable(
                                                                 NULL,
                                                                 0,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferWidthKey, width);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferHeightKey, height);
    CFDictionarySetBoolean(destinationPixelBufferAttributes,
                          kCVPixelBufferOpenGLESCompatibilityKey, YES);
    outputCallback.decompressionOutputCallback = VTDecoderCallback;
    outputCallback.decompressionOutputRefCon = context  ;
    status = VTDecompressionSessionCreate(
                                          kCFAllocatorDefault,
                                          context->fmt_desc.fmt_desc,
                                          NULL,
                                          destinationPixelBufferAttributes,
                                          &outputCallback,
                                          &vt_session);

    if (status != noErr) {
        NSError* error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
        NSLog(@"Error %@", [error description]);
        ALOGI("%s - failed with status = (%d)", __FUNCTION__, (int)status);
    }
    CFRelease(destinationPixelBufferAttributes);

    memset(&context->sample_info, 0, sizeof(struct sample_info));

    return vt_session;
}



static int decode_video_internal(Ijk_VideoToolBox_Opaque* context, AVCodecContext *avctx, const AVPacket *avpkt, int* got_picture_ptr)
{
    FFPlayer *ffp                   = context->ffp;
    OSStatus status                 = 0;
    uint32_t decoder_flags          = 0;
    sample_info *sample_info        = NULL;
    CMSampleBufferRef sample_buff   = NULL;
    AVIOContext *pb                 = NULL;
    int demux_size                  = 0;
    uint8_t *demux_buff             = NULL;
    uint8_t *pData                  = avpkt->data;
    int iSize                       = avpkt->size;
    double pts                      = avpkt->pts;
    double dts                      = avpkt->dts;

    if (!context) {
        goto failed;
    }

    if (context->refresh_session) {
        decoder_flags |= kVTDecodeFrame_DoNotOutputFrame;
        // ALOGI("flag :%d flag %d \n", decoderFlags,avpkt->flags);
    }

    if (context->refresh_request) {
        while (context->m_queue_depth > 0) {
            SortQueuePop(context);
        }

        vtbsession_destroy(context);
        memset(&context->sample_info, 0, sizeof(struct sample_info));

        context->vt_session = vtbsession_create(context);
        if (!context->vt_session)
            goto failed;
        context->refresh_request = false;
    }

    if (pts == AV_NOPTS_VALUE) {
        pts = dts;
    }

    if (context->fmt_desc.convert_bytestream) {
        // ALOGI("the buffer should m_convert_byte\n");
        if(avio_open_dyn_buf(&pb) < 0) {
            goto failed;
        }
        ff_avc_parse_nal_units(pb, pData, iSize);
        demux_size = avio_close_dyn_buf(pb, &demux_buff);
        // ALOGI("demux_size:%d\n", demux_size);
        if (demux_size == 0) {
            goto failed;
        }
        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, demux_buff, demux_size);
    } else if (context->fmt_desc.convert_3byteTo4byteNALSize) {
        // ALOGI("3byteto4byte\n");
        if (avio_open_dyn_buf(&pb) < 0) {
            goto failed;
        }

        uint32_t nal_size;
        uint8_t *end = avpkt->data + avpkt->size;
        uint8_t *nal_start = pData;
        while (nal_start < end) {
            nal_size = AV_RB24(nal_start);
            avio_wb32(pb, nal_size);
            nal_start += 3;
            avio_write(pb, nal_start, nal_size);
            nal_start += nal_size;
        }
        demux_size = avio_close_dyn_buf(pb, &demux_buff);
        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, demux_buff, demux_size);
    } else {
        sample_buff = CreateSampleBufferFrom(context->fmt_desc.fmt_desc, pData, iSize);
    }
    if (!sample_buff) {
        if (demux_size) {
            av_free(demux_buff);
        }
        ALOGI("%s - CreateSampleBufferFrom failed", __FUNCTION__);
        goto failed;
    }

    if (avpkt->flags & AV_PKT_FLAG_NEW_SEG) {
        context->new_seg_flag = true;
    }

    sample_info = &context->sample_info;
    if (!sample_info) {
        ALOGE("%s, failed to peek frame_info\n", __FUNCTION__);
        goto failed;
    }

    sample_info->pts    = pts;
    sample_info->dts    = dts;
    sample_info->serial = context->serial;
    sample_info->sar_num = avctx->sample_aspect_ratio.num;
    sample_info->sar_den = avctx->sample_aspect_ratio.den;

    status = VTDecompressionSessionDecodeFrame(context->vt_session, sample_buff, decoder_flags, (void*)sample_info, 0);
    if (status == noErr) {
        if (ffp->is->videoq.abort_request)
            goto failed;
    }

    if (status != 0) {

        ALOGE("decodeFrame %d %s\n", (int)status, vtb_get_error_string(status));

        if (status == kVTInvalidSessionErr) {
            context->refresh_session = true;
        }
        if (status == kVTVideoDecoderMalfunctionErr) {
            context->recovery_drop_packet = true;
            context->refresh_session = true;
        }
        goto failed;
    }



    if (sample_buff) {
        CFRelease(sample_buff);
    }
    if (demux_size) {
        av_free(demux_buff);
    }

    *got_picture_ptr = 1;
    return 0;
failed:
    if (sample_buff) {
        CFRelease(sample_buff);
    }
    if (demux_size) {
        av_free(demux_buff);
    }
    *got_picture_ptr = 0;
    return -1;
}

static inline void ResetPktBuffer(Ijk_VideoToolBox_Opaque* context) {
    for (int i = 0 ; i < context->m_buffer_deep; i++) {
        av_packet_unref(&context->m_buffer_packet[i]);
    }
    context->m_buffer_deep = 0;
    memset(context->m_buffer_packet, 0, sizeof(context->m_buffer_packet));
}

static inline void DuplicatePkt(Ijk_VideoToolBox_Opaque* context, const AVPacket* pkt) {
    if (context->m_buffer_deep >= MAX_PKT_QUEUE_DEEP) {
        context->idr_based_identified = false;
        ResetPktBuffer(context);
    }
    AVPacket* avpkt = &context->m_buffer_packet[context->m_buffer_deep];
    av_copy_packet(avpkt, pkt);
    context->m_buffer_deep++;
}



static int decode_video(Ijk_VideoToolBox_Opaque* context, AVCodecContext *avctx, AVPacket *avpkt, int* got_picture_ptr)
{
    int      ret            = 0;
    uint8_t *size_data      = NULL;
    int      size_data_size = 0;

    if (!avpkt || !avpkt->data) {
        return 0;
    }

    if (context->ffp->vtb_handle_resolution_change &&
        context->codecpar->codec_id == AV_CODEC_ID_H264) {
        size_data = av_packet_get_side_data(avpkt, AV_PKT_DATA_NEW_EXTRADATA, &size_data_size);
        // minimum avcC(sps,pps) = 7
        if (size_data && size_data_size > 7) {
            int             got_picture = 0;
            AVFrame        *frame      = av_frame_alloc();
            AVDictionary   *codec_opts = NULL;
            AVCodecContext *new_avctx  = avcodec_alloc_context3(avctx->codec);
            if (!new_avctx)
                return AVERROR(ENOMEM);

            avcodec_parameters_to_context(new_avctx, context->codecpar);
            av_freep(&new_avctx->extradata);
            new_avctx->extradata = av_mallocz(size_data_size + AV_INPUT_BUFFER_PADDING_SIZE);
            if (!new_avctx->extradata)
                return AVERROR(ENOMEM);
            memcpy(new_avctx->extradata, size_data, size_data_size);
            new_avctx->extradata_size = size_data_size;

            av_dict_set(&codec_opts, "threads", "1", 0);
            ret = avcodec_open2(new_avctx, avctx->codec, &codec_opts);
            av_dict_free(&codec_opts);
            if (ret < 0) {
                avcodec_free_context(&new_avctx);
                return ret;
            }

            ret = avcodec_decode_video2(new_avctx, frame, &got_picture, avpkt);
            if (ret < 0) {
                avcodec_free_context(&new_avctx);
                return ret;
            } else {
                if (context->codecpar->width  != new_avctx->width &&
                    context->codecpar->height != new_avctx->height) {
                    avcodec_parameters_from_context(context->codecpar, new_avctx);
                    context->refresh_request = true;
                }
            }

            av_frame_unref(frame);
            avcodec_free_context(&new_avctx);
        }
    } else {
        if (ff_avpacket_is_idr(avpkt) == true) {
            context->idr_based_identified = true;
        }
        if (ff_avpacket_i_or_idr(avpkt, context->idr_based_identified) == true) {
            ResetPktBuffer(context);
            context->recovery_drop_packet = false;
        }
        if (context->recovery_drop_packet == true) {
            return -1;
        }
    }

    DuplicatePkt(context, avpkt);

    if (context->refresh_session) {
        ret = 0;

        vtbsession_destroy(context);
        memset(&context->sample_info, 0, sizeof(struct sample_info));

        context->vt_session = vtbsession_create(context);
        if (!context->vt_session)
            return -1;

        if ((context->m_buffer_deep > 0) &&
            ff_avpacket_i_or_idr(&context->m_buffer_packet[0], context->idr_based_identified) == true ) {
            for (int i = 0; i < context->m_buffer_deep; i++) {
                AVPacket* pkt = &context->m_buffer_packet[i];
                ret = decode_video_internal(context, avctx, pkt, got_picture_ptr);
            }
        } else {
            context->recovery_drop_packet = true;
            ret = -1;
            ALOGE("recovery error!!!!\n");
        }
        context->refresh_session = false;
        return ret;
    }
    return decode_video_internal(context, avctx, avpkt, got_picture_ptr);
}


static void dict_set_string(CFMutableDictionaryRef dict, CFStringRef key, const char * value)
{
    CFStringRef string;
    string = CFStringCreateWithCString(NULL, value, kCFStringEncodingASCII);
    CFDictionarySetValue(dict, key, string);
    CFRelease(string);
}

static void dict_set_boolean(CFMutableDictionaryRef dict, CFStringRef key, BOOL value)
{
    CFDictionarySetValue(dict, key, value ? kCFBooleanTrue: kCFBooleanFalse);
}


static void dict_set_object(CFMutableDictionaryRef dict, CFStringRef key, CFTypeRef *value)
{
    CFDictionarySetValue(dict, key, value);
}

static void dict_set_data(CFMutableDictionaryRef dict, CFStringRef key, uint8_t * value, uint64_t length)
{
    CFDataRef data;
    data = CFDataCreate(NULL, value, (CFIndex)length);
    CFDictionarySetValue(dict, key, data);
    CFRelease(data);
}

static void dict_set_i32(CFMutableDictionaryRef dict, CFStringRef key,
                         int32_t value)
{
    CFNumberRef number;
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
    CFDictionarySetValue(dict, key, number);
    CFRelease(number);
}

static CMFormatDescriptionRef CreateFormatDescriptionFromCodecData(Uint32 format_id, int width, int height, const uint8_t *extradata, int extradata_size, uint32_t atom)
{
    CMFormatDescriptionRef fmt_desc = NULL;
    OSStatus status;

    CFMutableDictionaryRef par = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,&kCFTypeDictionaryValueCallBacks);
    CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    /* CVPixelAspectRatio dict */
    dict_set_i32(par, CFSTR ("HorizontalSpacing"), 0);
    dict_set_i32(par, CFSTR ("VerticalSpacing"), 0);
    /* SampleDescriptionExtensionAtoms dict */
    dict_set_data(atoms, CFSTR ("avcC"), (uint8_t *)extradata, extradata_size);

      /* Extensions dict */
    dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationBottomField"), "left");
    dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationTopField"), "left");
    dict_set_boolean(extensions, CFSTR("FullRangeVideo"), FALSE);
    dict_set_object(extensions, CFSTR ("CVPixelAspectRatio"), (CFTypeRef *) par);
    dict_set_object(extensions, CFSTR ("SampleDescriptionExtensionAtoms"), (CFTypeRef *) atoms);
    status = CMVideoFormatDescriptionCreate(NULL, format_id, width, height, extensions, &fmt_desc);

    CFRelease(extensions);
    CFRelease(atoms);
    CFRelease(par);

    if (status == 0)
        return fmt_desc;
    else
        return NULL;
}

void videotoolbox_sync_free(Ijk_VideoToolBox_Opaque* context)
{
    context->dealloced = true;

    while (context && context->m_queue_depth > 0) {
        SortQueuePop(context);
    }

    vtbsession_destroy(context);

    if (context) {
        ResetPktBuffer(context);
    }

    vtbformat_destroy(&context->fmt_desc);

    avcodec_parameters_free(&context->codecpar);
}

int videotoolbox_sync_decode_frame(Ijk_VideoToolBox_Opaque* context)
{
    FFPlayer *ffp = context->ffp;
    VideoState *is = ffp->is;
    Decoder *d = &is->viddec;
    int got_frame = 0;
    do {
        int ret = -1;
        if (is->abort_request || d->queue->abort_request) {
            return -1;
        }

        if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
            AVPacket pkt;
            do {
                if (d->queue->nb_packets == 0)
                    SDL_CondSignal(d->empty_queue_cond);
                ffp_video_statistic_l(ffp);
                if (ffp_packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0)
                    return -1;
                if (ffp_is_flush_packet(&pkt)) {
                    avcodec_flush_buffers(d->avctx);
                    context->refresh_request = true;
                    context->serial += 1;
                    d->finished = 0;
                    ALOGI("flushed last keyframe pts %lld \n",d->pkt.pts);
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            } while (ffp_is_flush_packet(&pkt) || d->queue->serial != d->pkt_serial);

            av_packet_split_side_data(&pkt);

            av_packet_unref(&d->pkt);
            d->pkt_temp = d->pkt = pkt;
            d->packet_pending = 1;
        }

        ret = decode_video(context, d->avctx, &d->pkt_temp, &got_frame);
        if (ret < 0) {
            d->packet_pending = 0;
        } else {
            d->pkt_temp.dts =
            d->pkt_temp.pts = AV_NOPTS_VALUE;
            if (d->pkt_temp.data) {
                if (d->avctx->codec_type != AVMEDIA_TYPE_AUDIO)
                    ret = d->pkt_temp.size;
                d->pkt_temp.data += ret;
                d->pkt_temp.size -= ret;
                if (d->pkt_temp.size <= 0)
                    d->packet_pending = 0;
            } else {
                if (!got_frame) {
                    d->packet_pending = 0;
                    d->finished = d->pkt_serial;
                }
            }
        }
    } while (!got_frame && !d->finished);
    return got_frame;
}

static void vtbformat_destroy(VTBFormatDesc *fmt_desc)
{
    if (!fmt_desc || !fmt_desc->fmt_desc)
        return;

    CFRelease(fmt_desc->fmt_desc);
    fmt_desc->fmt_desc = NULL;
}

static int vtbformat_init(VTBFormatDesc *fmt_desc, AVCodecParameters *codecpar)
{
    int width           = codecpar->width;
    int height          = codecpar->height;
    int level           = codecpar->level;
    int profile         = codecpar->profile;
    int sps_level       = 0;
    int sps_profile     = 0;
    int extrasize       = codecpar->extradata_size;
    int codec           = codecpar->codec_id;
    uint8_t* extradata  = codecpar->extradata;

#if 0
    switch (profile) {
        case FF_PROFILE_H264_HIGH_10:
            if ([IJKDeviceModel currentModel].rank >= kIJKDeviceRank_AppleA7Class) {
                // Apple A7 SoC
                // Hi10p can be decoded into NV12 ('420v')
                break;
            }
        case FF_PROFILE_H264_HIGH_10_INTRA:
        case FF_PROFILE_H264_HIGH_422:
        case FF_PROFILE_H264_HIGH_422_INTRA:
        case FF_PROFILE_H264_HIGH_444_PREDICTIVE:
        case FF_PROFILE_H264_HIGH_444_INTRA:
        case FF_PROFILE_H264_CAVLC_444:
            goto failed;
    }
#endif
    if (width < 0 || height < 0) {
        goto fail;
    }

    switch (codec) {
        case AV_CODEC_ID_H264:
            if (extrasize < 7 || extradata == NULL) {
                ALOGI("%s - avcC atom too data small or missing", __FUNCTION__);
                goto fail;
            }

            if (extradata[0] == 1) {
                if (!validate_avcC_spc(extradata, extrasize, &fmt_desc->max_ref_frames, &sps_level, &sps_profile)) {
                    //goto failed;
                }
                if (level == 0 && sps_level > 0)
                    level = sps_level;

                if (profile == 0 && sps_profile > 0)
                    profile = sps_profile;
                if (profile == FF_PROFILE_H264_MAIN && level == 32 && fmt_desc->max_ref_frames > 4) {
                    ALOGE("%s - Main@L3.2 detected, VTB cannot decode with %d ref frames", __FUNCTION__, fmt_desc->max_ref_frames);
                    goto fail;
                }

                if (extradata[4] == 0xFE) {
                    extradata[4] = 0xFF;
                    fmt_desc->convert_3byteTo4byteNALSize = true;
                }

                fmt_desc->fmt_desc = CreateFormatDescriptionFromCodecData(kCMVideoCodecType_H264, width, height, extradata, extrasize,  IJK_VTB_FCC_AVCC);
                if (fmt_desc->fmt_desc == NULL) {
                    goto fail;
                }

                ALOGI("%s - using avcC atom of size(%d), ref_frames(%d)", __FUNCTION__, extrasize, fmt_desc->max_ref_frames);
            } else {
                if ((extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 0 && extradata[3] == 1) ||
                    (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 1)) {
                    AVIOContext *pb;
                    if (avio_open_dyn_buf(&pb) < 0) {
                        goto fail;
                    }

                    fmt_desc->convert_bytestream = true;
                    ff_isom_write_avcc(pb, extradata, extrasize);
                    extradata = NULL;

                    extrasize = avio_close_dyn_buf(pb, &extradata);

                    if (!validate_avcC_spc(extradata, extrasize, &fmt_desc->max_ref_frames, &sps_level, &sps_profile)) {
                        av_free(extradata);
                        goto fail;
                    }

                    fmt_desc->fmt_desc = CreateFormatDescriptionFromCodecData(kCMVideoCodecType_H264, width, height, extradata, extrasize, IJK_VTB_FCC_AVCC);
                    if (fmt_desc->fmt_desc == NULL) {
                        goto fail;
                    }

                    av_free(extradata);
                } else {
                    ALOGI("%s - invalid avcC atom data", __FUNCTION__);
                    goto fail;
                }
            }
            break;
        default:
            goto fail;
    }

    fmt_desc->max_ref_frames = FFMAX(fmt_desc->max_ref_frames, 2);
    fmt_desc->max_ref_frames = FFMIN(fmt_desc->max_ref_frames, 5);

    ALOGI("m_max_ref_frames %d \n", fmt_desc->max_ref_frames);

    return 0;
fail:
    vtbformat_destroy(fmt_desc);
    return -1;
}

Ijk_VideoToolBox_Opaque* videotoolbox_sync_create(FFPlayer* ffp, AVCodecContext* avctx)
{
    int ret = 0;

    if (ret) {
        ALOGW("%s - videotoolbox can not exists twice at the same time", __FUNCTION__);
        return NULL;
    }

    Ijk_VideoToolBox_Opaque *context_vtb = (Ijk_VideoToolBox_Opaque *)mallocz(sizeof(Ijk_VideoToolBox_Opaque));

    if (!context_vtb) {
        goto fail;
    }

    context_vtb->codecpar = avcodec_parameters_alloc();
    if (!context_vtb->codecpar)
        goto fail;

    ret = avcodec_parameters_from_context(context_vtb->codecpar, avctx);
    if (ret)
        goto fail;

    context_vtb->ffp = ffp;
    context_vtb->idr_based_identified = true;

    ret = vtbformat_init(&context_vtb->fmt_desc, context_vtb->codecpar);
    if (ret)
        goto fail;
    assert(context_vtb->fmt_desc.fmt_desc);
    vtbformat_destroy(&context_vtb->fmt_desc);

    context_vtb->vt_session = vtbsession_create(context_vtb);
    if (context_vtb->vt_session == NULL)
        goto fail;

    context_vtb->m_sort_queue = 0;
    context_vtb->m_queue_depth = 0;

    SDL_SpeedSamplerReset(&context_vtb->sampler);
    return context_vtb;

fail:
    videotoolbox_sync_free(context_vtb);
    return NULL;
}
