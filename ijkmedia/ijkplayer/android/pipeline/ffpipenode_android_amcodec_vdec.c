/*
 * ffpipenode_android_amcodec_vdec.c
 *
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ffpipenode_android_mediacodec_vdec.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_vout_android_nativewindow.h"
#include "ijkplayer/ff_ffpipenode.h"
#include "ijkplayer/ff_ffplay.h"
#include "ijkplayer/ff_ffplay_debug.h"
#include "ffpipeline_android.h"
#include "h264_nal.h"
#include "hevc_nal.h"

#include "amcodec/codec.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

// Emit extremely verbose timing debug logs.
#define TIMING_DEBUG              0

// 90kHz is an amlogic magic value, all timings are specified in a 90KHz clock.
#define AM_TIME_BASE              90000
#define AM_TIME_BASE_Q            (AVRational){1, AM_TIME_BASE}

// amcodec only tracks time using a 32 bit mask
#define AM_TIME_BITS              (32ULL)
#define AM_TIME_MASK              (0xffffffffULL)

// Framerate specified in a 96KHz clock
#define FPS_FREQ                  96000
#define AV_SYNC_THRESH            AM_TIME_BASE*30

// am_codec_set_ctnl_mode values.
#define TRICKMODE_NONE            0x00
#define TRICKMODE_I               0x01
#define TRICKMODE_FFFB            0x02




// same as AV_NOPTS_VALUE
#define INT64_0                   INT64_C(0x8000000000000000)

#define EXTERNAL_PTS              (1)
#define SYNC_OUTSIDE              (2)

// missing tags
#define CODEC_TAG_VC_1            (0x312D4356)
#define CODEC_TAG_RV30            (0x30335652)
#define CODEC_TAG_RV40            (0x30345652)
#define CODEC_TAG_MJPEG           (0x47504a4d)
#define CODEC_TAG_mjpeg           (0x47504a4c)
#define CODEC_TAG_jpeg            (0x6765706a)
#define CODEC_TAG_mjpa            (0x61706a6d)

#define RW_WAIT_TIME              (20 * 1000) // 20ms

// Errors from amcodec
#define P_PRE                     (0x02000000)
#define F_PRE                     (0x03000000)
#define PLAYER_SUCCESS            (0)
#define PLAYER_FAILED             (-(P_PRE|0x01))
#define PLAYER_NOMEM              (-(P_PRE|0x02))
#define PLAYER_EMPTY_P            (-(P_PRE|0x03))

#define PLAYER_WR_FAILED          (-(P_PRE|0x21))
#define PLAYER_WR_EMPTYP          (-(P_PRE|0x22))
#define PLAYER_WR_FINISH          (P_PRE|0x1)

#define PLAYER_PTS_ERROR          (-(P_PRE|0x31))
#define PLAYER_UNSUPPORT          (-(P_PRE|0x35))
#define PLAYER_CHECK_CODEC_ERROR  (-(P_PRE|0x39))


#define HDR_BUF_SIZE 1024

typedef struct hdr_buf {
    char *data;
    int size;
} hdr_buf_t;

typedef struct am_packet {
    AVPacket      avpkt;
    int64_t       avpts;
    int64_t       avdts;
    int           avduration;
    int           isvalid;
    int           newflag;
    int64_t       lastpts;
    unsigned char *data;
    unsigned char *buf;
    int           data_size;
    int           buf_size;
    hdr_buf_t     *hdr;
    codec_para_t  *codec;
} am_packet_t;


typedef enum {
    AM_STREAM_UNKNOWN = 0,
    AM_STREAM_TS,
    AM_STREAM_PS,
    AM_STREAM_ES,
    AM_STREAM_RM,
    AM_STREAM_AUDIO,
    AM_STREAM_VIDEO,
} pstream_type;

typedef struct IJKAM_Pipenode_Opaque {
    FFPlayer            *ffp;
    Decoder             *decoder;

    pstream_type        stream_type;
    vformat_t           video_format;
    unsigned int        video_codec_id;
    unsigned int        video_codec_tag;
    unsigned int        video_codec_type;
    int                 extrasize;
    uint8_t             *extradata;
    int                 video_width;
    int                 video_height;
    codec_para_t        vcodec;
    int                 check_first_pts;

    int                 paused;

    int64_t             decode_dts;
    int64_t             decode_pts;
    int                 do_discontinuity_check;

    size_t              nal_size;

    am_packet_t         am_pkt;

    int                 input_packet_count;
} IJKAM_Pipenode_Opaque;

// Alias for IJKAM_Pipenode_Opaque because I've copy and pasted lots of code that needs am_private_t.
typedef IJKAM_Pipenode_Opaque am_private_t;


// libamplayer.so entry points
static int (*am_codec_init)(codec_para_t *pcodec);
static int (*am_codec_close)(codec_para_t *pcodec);
static int (*am_codec_reset)(codec_para_t *pcodec);
static int (*am_codec_pause)(codec_para_t *pcodec);
static int (*am_codec_resume)(codec_para_t *pcodec);
static int (*am_codec_write)(codec_para_t *pcodec, void *buffer, int len);
static int (*am_codec_checkin_pts)(codec_para_t *pcodec, unsigned long pts);
static int (*am_codec_get_vbuf_state)(codec_para_t *pcodec, struct buf_status *buf);
static int (*am_codec_get_vdec_state)(codec_para_t *pcodec, struct vdec_status *vdec);
static int (*am_codec_init_cntl)(codec_para_t *pcodec);
static int (*am_codec_poll_cntl)(codec_para_t *pcodec);
static int (*am_codec_set_cntl_mode)(codec_para_t *pcodec, unsigned int mode);
static int (*am_codec_set_cntl_avthresh)(codec_para_t *pcodec, unsigned int avthresh);
static int (*am_codec_set_cntl_syncthresh)(codec_para_t *pcodec, unsigned int syncthresh);
static int (*am_codec_get_vpts)(codec_para_t *pcodec, unsigned long *out);


// forward decls
static void func_destroy(IJKFF_Pipenode *node);
static int func_flush(IJKFF_Pipenode *node);
static int func_run_sync(IJKFF_Pipenode *node);
static int pre_header_feeding(am_private_t *para, am_packet_t *pkt);
static int set_header_info(am_private_t *para);
static int write_av_packet(am_private_t *para, am_packet_t *pkt);
static void syncToMaster();
static int reset(IJKAM_Pipenode_Opaque *opaque );

static int amlogic_not_present = 0;

static void * find_symbol(const char * sym)
{
  void * addr = dlsym(RTLD_DEFAULT, sym);
  if(!addr)
    amlogic_not_present = 1;
  return addr;
}


static void loadLibrary()
{
    // We use Java to call System.loadLibrary as dlopen is weird on Android.
    void * lib = dlopen("libamplayer.so", RTLD_NOW);
    if(!lib) {
      ALOGD("amplayer library did not load.");
      amlogic_not_present = 1;
      return;
    }

    am_codec_init                = find_symbol("codec_init");
    am_codec_close               = find_symbol("codec_close");
    am_codec_reset               = find_symbol("codec_reset");
    am_codec_pause               = find_symbol("codec_pause");
    am_codec_resume              = find_symbol("codec_resume");
    am_codec_write               = find_symbol("codec_write");
    am_codec_checkin_pts         = find_symbol("codec_checkin_pts");
    am_codec_get_vbuf_state      = find_symbol("codec_get_vbuf_state");
    am_codec_get_vdec_state      = find_symbol("codec_get_vdec_state");
    am_codec_init_cntl           = find_symbol("codec_init_cntl");
    am_codec_poll_cntl           = find_symbol("codec_poll_cntl");
    am_codec_set_cntl_mode       = find_symbol("codec_set_cntl_mode");
    am_codec_set_cntl_avthresh   = find_symbol("codec_set_cntl_avthresh");
    am_codec_set_cntl_syncthresh = find_symbol("codec_set_cntl_syncthresh");
    am_codec_get_vpts            = find_symbol("codec_get_vpts");
}

static void am_packet_init(am_packet_t *pkt, codec_para_t *codec)
{
    memset(&pkt->avpkt, 0, sizeof(AVPacket));
    pkt->avpts      = 0;
    pkt->avdts      = 0;
    pkt->avduration = 0;
    pkt->isvalid    = 0;
    pkt->newflag    = 0;
    pkt->lastpts    = 0;
    pkt->data       = NULL;
    pkt->buf        = NULL;
    pkt->data_size  = 0;
    pkt->buf_size   = 0;
    pkt->hdr        = NULL;
    pkt->codec      = codec;
}

void am_packet_release(am_packet_t *pkt)
{
  if (pkt->buf != NULL)
    free(pkt->buf), pkt->buf= NULL;
  if (pkt->hdr != NULL)
  {
    if (pkt->hdr->data != NULL)
      free(pkt->hdr->data), pkt->hdr->data = NULL;
    free(pkt->hdr), pkt->hdr = NULL;
  }

  //pkt->codec = NULL;
}

static int sysfs_setint(const char * path, const int val)
{
  int fd = open(path, O_RDWR, 0644);
  int ret = 0;
  if (fd >= 0)
  {
    char bcmd[16];
    sprintf(bcmd, "%d", val);
    if (write(fd, bcmd, strlen(bcmd)) < 0)
      ret = -1;
    close(fd);
  }
  if (ret)
    ALOGE("%s: error writing %s",__FUNCTION__, path);

  return ret;
}

#if 0
static int sysfs_getint(const char * path, int* val)
{
  int fd = open(path, O_RDONLY);
  int ret = 0;
  if (fd >= 0)
  {
    char bcmd[16];
    if (read(fd, bcmd, sizeof(bcmd)) < 0)
      ret = -1;
    else
      *val = strtol(bcmd, NULL, 16);

    close(fd);
  }
  if (ret)
    ALOGE("%s: error reading %s",__FUNCTION__, path);

  return ret;
}
#endif

static int64_t get_pts_video() {
    int fd = open("/sys/class/tsync/pts_video", O_RDONLY);
    if (fd >= 0) {
        char pts_str[16];
        int size = read(fd, pts_str, sizeof(pts_str));
        close(fd);
        if (size > 0) {
            unsigned long pts = strtoul(pts_str, NULL, 16);
            return pts;
        }
    }

    ALOGE("get_pts_video: open /tsync/event error");
    return -1;
}

static int64_t am_time_sub(int64_t a, int64_t b)
{
    int64_t shift = 64 - AM_TIME_BITS;
    return ((a - b) << shift) >> shift;
}

static int64_t get_pts_pcrscr() {
    int fd = open("/sys/class/tsync/pts_pcrscr", O_RDONLY);
    if (fd >= 0) {
        char pts_str[16];
        int size = read(fd, pts_str, sizeof(pts_str));
        close(fd);
        if (size > 0) {
            unsigned long pts = strtoul(pts_str, NULL, 16);
            return pts;
        }
    }

    ALOGE("get_pts_pcrscr: open /tsync/event error");
    return -1;
}

static int set_pts_pcrscr(int64_t value)
{
  int fd = open("/sys/class/tsync/pts_pcrscr", O_WRONLY);
  if (fd >= 0)
  {
    char pts_str[64];
    unsigned long pts = (unsigned long)value;
    sprintf(pts_str, "0x%lx", pts);
    write(fd, pts_str, strlen(pts_str));
    close(fd);
    return 0;
  }

  ALOGE("set_pts_pcrscr: open pts_pcrscr error");
  return -1;
}

static void set_clock_at(Clock *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

#if 0
static double get_clock(Clock *c)
{
    if (*c->queue_serial != c->serial)
        return NAN;
    if (c->paused) {
        return c->pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
    }
}

static int get_master_sync_type(VideoState *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

/* get the current master clock value */
static double get_master_clock(VideoState *is)
{
    double val;

    switch (get_master_sync_type(is)) {
        case AV_SYNC_VIDEO_MASTER:
            val = get_clock(&is->vidclk);
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = get_clock(&is->audclk);
            break;
        default:
            val = get_clock(&is->extclk);
            break;
    }
    return val;
}
#endif

static vformat_t codecid_to_vformat(enum AVCodecID id)
{
  vformat_t format;
  switch (id)
  {
    case AV_CODEC_ID_MPEG1VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO:
    case AV_CODEC_ID_MPEG2VIDEO_XVMC:
      format = VFORMAT_MPEG12;
      break;
    case AV_CODEC_ID_H263:
    case AV_CODEC_ID_MPEG4:
    case AV_CODEC_ID_H263P:
    case AV_CODEC_ID_H263I:
    case AV_CODEC_ID_MSMPEG4V2:
    case AV_CODEC_ID_MSMPEG4V3:
    case AV_CODEC_ID_FLV1:
      format = VFORMAT_MPEG4;
      break;
    case AV_CODEC_ID_RV10:
    case AV_CODEC_ID_RV20:
    case AV_CODEC_ID_RV30:
    case AV_CODEC_ID_RV40:
      format = VFORMAT_REAL;
      break;
    case AV_CODEC_ID_H264:
      format = VFORMAT_H264;
      break;
    /*
    case AV_CODEC_ID_H264MVC:
      // H264 Multiview Video Coding (3d blurays)
      format = VFORMAT_H264MVC;
      break;
    */
    case AV_CODEC_ID_MJPEG:
      format = VFORMAT_MJPEG;
      break;
    case AV_CODEC_ID_VC1:
    case AV_CODEC_ID_WMV3:
      format = VFORMAT_VC1;
      break;
    case AV_CODEC_ID_AVS:
    case AV_CODEC_ID_CAVS:
      format = VFORMAT_AVS;
      break;
    case AV_CODEC_ID_HEVC:
      format = VFORMAT_HEVC;
      break;

    default:
      format = VFORMAT_UNSUPPORT;
      break;
  }

  ALOGD("codecid_to_vformat, id(%d) -> vformat(%d)", (int)id, format);
  return format;
}

static vdec_type_t codec_tag_to_vdec_type(unsigned int codec_tag)
{
  vdec_type_t dec_type;
  switch (codec_tag)
  {
    case CODEC_TAG_MJPEG:
    case CODEC_TAG_mjpeg:
    case CODEC_TAG_jpeg:
    case CODEC_TAG_mjpa:
      // mjpeg
      dec_type = VIDEO_DEC_FORMAT_MJPEG;
      break;
    case CODEC_TAG_XVID:
    case CODEC_TAG_xvid:
    case CODEC_TAG_XVIX:
      // xvid
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_COL1:
    case CODEC_TAG_DIV3:
    case CODEC_TAG_MP43:
      // divx3.11
      dec_type = VIDEO_DEC_FORMAT_MPEG4_3;
      break;
    case CODEC_TAG_DIV4:
    case CODEC_TAG_DIVX:
      // divx4
      dec_type = VIDEO_DEC_FORMAT_MPEG4_4;
      break;
    case CODEC_TAG_DIV5:
    case CODEC_TAG_DX50:
    case CODEC_TAG_M4S2:
    case CODEC_TAG_FMP4:
      // divx5
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_DIV6:
      // divx6
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case CODEC_TAG_MP4V:
    case CODEC_TAG_RMP4:
    case CODEC_TAG_MPG4:
    case CODEC_TAG_mp4v:
    case AV_CODEC_ID_MPEG4:
      // mp4
      dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
      break;
    case AV_CODEC_ID_H263:
    case CODEC_TAG_H263:
    case CODEC_TAG_h263:
    case CODEC_TAG_s263:
    case CODEC_TAG_F263:
      // h263
      dec_type = VIDEO_DEC_FORMAT_H263;
      break;
    case CODEC_TAG_AVC1:
    case CODEC_TAG_avc1:
    case CODEC_TAG_H264:
    case CODEC_TAG_h264:
    case AV_CODEC_ID_H264:
      // h264
      dec_type = VIDEO_DEC_FORMAT_H264;
      break;
    /*
    case AV_CODEC_ID_H264MVC:
      dec_type = VIDEO_DEC_FORMAT_H264;
      break;
    */
    case AV_CODEC_ID_RV30:
    case CODEC_TAG_RV30:
      // realmedia 3
      dec_type = VIDEO_DEC_FORMAT_REAL_8;
      break;
    case AV_CODEC_ID_RV40:
    case CODEC_TAG_RV40:
      // realmedia 4
      dec_type = VIDEO_DEC_FORMAT_REAL_9;
      break;
    case CODEC_TAG_WMV3:
      // wmv3
      dec_type = VIDEO_DEC_FORMAT_WMV3;
      break;
    case AV_CODEC_ID_VC1:
    case CODEC_TAG_VC_1:
    case CODEC_TAG_WVC1:
    case CODEC_TAG_WMVA:
      // vc1
      dec_type = VIDEO_DEC_FORMAT_WVC1;
      break;
    case AV_CODEC_ID_VP6F:
      // vp6
      dec_type = VIDEO_DEC_FORMAT_SW;
      break;
    case AV_CODEC_ID_CAVS:
    case AV_CODEC_ID_AVS:
      // avs
      dec_type = VIDEO_DEC_FORMAT_AVS;
      break;
    case AV_CODEC_ID_HEVC:
      // h265
      dec_type = VIDEO_DEC_FORMAT_HEVC;
      break;
    default:
      dec_type = VIDEO_DEC_FORMAT_UNKNOW;
      break;
  }

  ALOGD("codec_tag_to_vdec_type, codec_tag(%d) -> vdec_type(%d)", codec_tag, dec_type);
  return dec_type;
}




IJKFF_Pipenode *ffpipenode_create_video_decoder_from_android_amcodec(FFPlayer *ffp)
{
    if (!ffp || !ffp->is)
        return NULL;

    ALOGI("amcodec initializing.");

    loadLibrary();

    if(amlogic_not_present)
        return NULL;

    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKAM_Pipenode_Opaque));
    if (!node)
        return node;

    VideoState            *is     = ffp->is;

    IJKAM_Pipenode_Opaque *opaque = node->opaque;
    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    node->func_flush    = func_flush;

    //opaque->pipeline    = pipeline;
    opaque->ffp         = ffp;
    opaque->decoder     = &is->viddec;
    //opaque->weak_vout   = vout;

    opaque->check_first_pts = 0;

    opaque->paused = false;

    AVCodecContext * avctx = opaque->decoder->avctx;

    opaque->stream_type = AM_STREAM_ES;

    opaque->video_codec_id = avctx->codec_id; //AV_CODEC_ID_H264;
    opaque->video_codec_tag = avctx->codec_tag; //CODEC_TAG_AVC1;

    opaque->video_format = codecid_to_vformat(avctx->codec_id); //VFORMAT_H264
    if(opaque->video_format == VFORMAT_UNSUPPORT)
      return NULL;

    opaque->video_codec_type = codec_tag_to_vdec_type(avctx->codec_tag);  //VIDEO_DEC_FORMAT_H264;
    //if(opaque->video_codec_type == VIDEO_DEC_FORMAT_UNKNOW)
    //  return NULL;

    opaque->extrasize = avctx->extradata_size;
    opaque->extradata = (uint8_t*)malloc(opaque->extrasize);
    memcpy(opaque->extradata, avctx->extradata, opaque->extrasize);
    opaque->video_width = avctx->width;
    opaque->video_height = avctx->height;

    codec_para_t * vcodec = &opaque->vcodec;
    memset(vcodec, 0, sizeof(codec_para_t));

    vcodec->handle = -1;        ///< codec device handler
    vcodec->cntl_handle = -1;   ///< video control device handler
    vcodec->sub_handle = -1;    ///< subtile device handler
    vcodec->stream_type = STREAM_TYPE_ES_VIDEO;  ///< stream type(es, ps, rm, ts)
    vcodec->has_video = 1;   ///< stream has video(1) or not(0)
    vcodec->has_audio = 0;   ///< stream has audio(1) or not(0)
    vcodec->has_sub = 0;     ///< stream has subtitle(1) or not(0)
    vcodec->noblock = 0;     ///< codec device is NONBLOCK(1) or not(0)
    vcodec->audio_type = 0;             ///< stream audio type(PCM, WMA...)
    vcodec->sub_type = 0;               ///< stream subtitle type(TXT, SSA...)
    vcodec->video_pid = -1;              ///< stream video pid
    vcodec->audio_pid = 0;              ///< stream audio pid
    vcodec->sub_pid = 0;                ///< stream subtitle pid
    vcodec->audio_channels = 0;         ///< stream audio channel number
    vcodec->audio_samplerate = 0;       ///< steram audio sample rate
    vcodec->vbuf_size = 0;              ///< video buffer size of codec device
    vcodec->abuf_size= 0;              ///< audio buffer size of codec device
    vcodec->am_sysinfo.width = avctx->width;         // am_sysinfo, unsigned int width
    vcodec->am_sysinfo.height = avctx->height;        // am_sysinfo, unsigned int height
    vcodec->am_sysinfo.rate = FPS_FREQ / 30;          // am_sysinfo, unsigned int rate
    vcodec->am_sysinfo.extra = 0;         // am_sysinfo, unsigned int extra
    vcodec->am_sysinfo.status = 0;        // am_sysinfo, unsigned int status
    vcodec->am_sysinfo.param = NULL; // am_sysinfo, unsigned int param
    vcodec->am_sysinfo.ratio = 0x00010001;         // am_sysinfo, unsigned int ratio
    vcodec->am_sysinfo.ratio64 = 0x0000000100000001ULL;         // am_sysinfo, unsigned int ratio


    vcodec->video_type = opaque->video_format;             //VFORMAT_H264 ///< stream video type(H264, VC1...)
    vcodec->am_sysinfo.format = opaque->video_codec_type; //VIDEO_DEC_FORMAT_H264;   ///< system information for video
    vcodec->am_sysinfo.param = (void *)(EXTERNAL_PTS);
    //vcodec->am_sysinfo.param = (void *)(EXTERNAL_PTS | SYNC_OUTSIDE);

    int ret = am_codec_init(vcodec);
    if (ret != CODEC_ERROR_NONE)
    {
      ALOGD("ffpipenode_create_video_decoder_from_android_amcodec codec init failed, ret=0x%x", -ret);
      return false;
    }

    // make sure we are not stuck in pause (amcodec bug)
    ret = am_codec_resume(vcodec);
    ret = am_codec_set_cntl_mode(vcodec, TRICKMODE_NONE);

    ret = am_codec_set_cntl_avthresh(vcodec, AV_SYNC_THRESH);
    ret = am_codec_set_cntl_syncthresh(vcodec, 0);

    opaque->decode_dts = 0;
    opaque->decode_pts = 0;

    am_packet_init(&opaque->am_pkt, vcodec);



    // avc1 is the codec tag when h264 is embedded in an mp4 and needs the stupid
    // nal_size and extradata stuff processed.
    if(avctx->codec_tag == CODEC_TAG_AVC1 || avctx->codec_tag == CODEC_TAG_avc1 ||
       avctx->codec_tag == CODEC_TAG_hvc1 || avctx->codec_tag == CODEC_TAG_hev1) {

        ALOGD("stream is avc1/hvc1, fixing sps/pps. extrasize:%d", avctx->extradata_size);

        size_t   sps_pps_size   = 0;
        size_t   convert_size   = avctx->extradata_size + 200;
        uint8_t *convert_buffer = (uint8_t *)calloc(1, convert_size);
        if (!convert_buffer) {
            ALOGE("%s:sps_pps_buffer: alloc failed\n", __func__);
            return NULL;
        }

        if(avctx->codec_tag == CODEC_TAG_AVC1 || avctx->codec_tag == CODEC_TAG_avc1) {
            if (0 != convert_sps_pps(avctx->extradata, avctx->extradata_size,
                                     convert_buffer, convert_size,
                                     &sps_pps_size, &opaque->nal_size)) {
                ALOGE("%s:convert_sps_pps: failed\n", __func__);
                return NULL;
            }
        } else {
            if (0 != convert_hevc_nal_units(avctx->extradata, avctx->extradata_size,
                                     convert_buffer, convert_size,
                                     &sps_pps_size, &opaque->nal_size)) {
                ALOGE("%s:convert_hevc_nal_units: failed\n", __func__);
                return NULL;
            }
        }
        free(opaque->extradata);
        opaque->extrasize = sps_pps_size;
        opaque->extradata = convert_buffer;
    }



    ret = pre_header_feeding(opaque, &opaque->am_pkt);

    // disable tsync, we are playing video disconnected from audio
    sysfs_setint("/sys/class/tsync/enable", 0);
    sysfs_setint("/sys/class/video/screen_mode", 1);
    sysfs_setint("/sys/class/video/disable_video", 0);

    ffp->stat.vdec_type = FFP_PROPV_DECODER_AMLOGIC;

    return node;
}


static int feed_input_buffer(JNIEnv *env, IJKFF_Pipenode *node, int *enqueue_count)
{
    IJKAM_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    int                    ret      = 0;

    AVCodecContext * avctx = opaque->decoder->avctx;
    am_packet_t * am_pkt = &opaque->am_pkt;

    if (enqueue_count)
        *enqueue_count = 0;

    if (d->queue->abort_request) {
        ret = 0;
        goto fail;
    }

#if 1
    if (is->paused) {
        if(!opaque->paused) {
            ALOGD("amcodec pausing!");
            am_codec_pause(&opaque->vcodec);
            am_codec_set_cntl_mode(&opaque->vcodec, TRICKMODE_NONE);
            opaque->paused = true;
        }

        usleep(1000000 / 30);   // Hmmm, is there a condwait for resuming?
        return 0;
    } else {
        if(opaque->paused) {
            ALOGE("amcodec resuming!");
            am_codec_resume(&opaque->vcodec);
            am_codec_set_cntl_mode(&opaque->vcodec, TRICKMODE_NONE);
            opaque->paused = false;
        }
    }
#endif

    // pull in a new packet if nothing pending
    if (!d->packet_pending || d->queue->serial != d->pkt_serial) {
        AVPacket pkt;
        do {
            if (d->queue->nb_packets == 0)
                SDL_CondSignal(d->empty_queue_cond);
            if (ffp_packet_queue_get_or_buffering(ffp, d->queue, &pkt, &d->pkt_serial, &d->finished) < 0) {
                ret = -1;
                goto fail;
            }
            if (ffp_is_flush_packet(&pkt)) {
                ALOGD("amcodec flush packet");

                reset(opaque);

                // request flush before lock, or never get mutex
                d->finished = 0;
                d->next_pts = d->start_pts;
                d->next_pts_tb = d->start_pts_tb;
            }
        } while (ffp_is_flush_packet(&pkt) || d->queue->serial != d->pkt_serial);
        av_packet_unref(&d->pkt);
        d->pkt_temp = d->pkt = pkt;
        d->packet_pending = 1;

        if (d->pkt_temp.data) {
            *enqueue_count += 1;
            H264ConvertState convert_state = {0, 0};

            if (opaque->nal_size > 0 && (avctx->codec_id == AV_CODEC_ID_H264 || avctx->codec_id == AV_CODEC_ID_HEVC)) {
                convert_h264_to_annexb(d->pkt_temp.data, d->pkt_temp.size, opaque->nal_size, &convert_state);
            }
        }
    }

    if (d->pkt_temp.data) {
        *enqueue_count += 1;

        // amlogic uses an int inside the kernel and we need to copy that.
        int64_t dts = d->pkt_temp.dts;
        int64_t pts = d->pkt_temp.pts;

        // we need to force pts to be in the same 'phase' as dts
        //if(pts != AV_NOPTS_VALUE)
        //  pts = (((int)pts-(int)dts)&0x7fffffff) + dts;

        if (dts != AV_NOPTS_VALUE)
            dts = av_rescale_q(dts, is->video_st->time_base, AM_TIME_BASE_Q);

        if (pts != AV_NOPTS_VALUE)
            pts = av_rescale_q(pts, is->video_st->time_base, AM_TIME_BASE_Q);

        if(dts > pts)
            dts = pts;

        if(dts <= opaque->decode_dts)
            dts = opaque->decode_dts + 1;

        if (pts == AV_NOPTS_VALUE)
          pts = dts;

        if (opaque->do_discontinuity_check) {

          // on a discontinuity we need to wait for all frames to popout of the decoder.
          bool discontinuity_dts = opaque->decode_dts != 0 && abs(opaque->decode_dts - dts) > AM_TIME_BASE;
          bool discontinuity_pts = opaque->decode_pts != 0 && abs(opaque->decode_pts - pts) > AM_TIME_BASE;
          if(discontinuity_dts) {
            int64_t pts_pcrscr = get_pts_pcrscr();

            // wait for pts_scr to hit the last decode_dts
            while(am_time_sub(pts_pcrscr, opaque->decode_dts) < 0) {
              ALOGD("clock dts discontinuity: ptsscr:%jd decode_dts:%jd dts:%jd", pts_pcrscr, opaque->decode_dts, dts);

              usleep(1000000/30);

              pts_pcrscr = get_pts_pcrscr();
            }

            reset(opaque);
          } else if(discontinuity_pts) {
            int64_t pts_pcrscr = get_pts_pcrscr();

            // wait for pts_scr to hit the last decode_pts
            while(am_time_sub(pts_pcrscr, opaque->decode_pts) < 0) {
              ALOGD("clock pts discontinuity: ptsscr:%jd decode_pts:%jd next_pts:%jd", pts_pcrscr, opaque->decode_pts, pts);

              usleep(1000000/30);

              pts_pcrscr = get_pts_pcrscr();
            }

            reset(opaque);
          }
        }


        opaque->decode_dts = dts;
        opaque->decode_pts = pts;

#if TIMING_DEBUG
        ALOGD("queued dts:%llu pts:%llu\n", dts, pts);
#endif

        if (!is->viddec.first_frame_decoded) {
            ALOGD("Video: first frame decoded\n");
            is->viddec.first_frame_decoded_time = SDL_GetTickHR();
            is->viddec.first_frame_decoded = 1;

            double time = av_gettime_relative() / 1000000.0;
            set_clock_at(&is->vidclk, pts / 90000.0, d->pkt_serial, time);
        }

        //if((pts - last_pts) > 300

        //if(pts_jump) {
        //    if (pts != AV_NOPTS_VALUE) {
        //        double time = av_gettime_relative() / 1000000.0;
        //        set_clock_at(&is->vidclk, pts / 90000.0, d->pkt_serial, time);
        //    }
        //}

        am_pkt->data       = d->pkt_temp.data;
        am_pkt->data_size  = d->pkt_temp.size;
        am_pkt->newflag    = 1;
        am_pkt->isvalid    = 1;
        am_pkt->avduration = 0;
        am_pkt->avdts      = dts;
        am_pkt->avpts      = pts;

        // some formats need header/data tweaks.
        // the actual write occurs once in write_av_packet
        // and is controlled by am_pkt.newflag.
        set_header_info(opaque);

        // loop until we write all into codec, am_pkt.isvalid
        // will get set to zero once everything is consumed.
        // PLAYER_SUCCESS means all is ok, not all bytes were written.
        int loop = 0;
        while (opaque->am_pkt.isvalid && loop < 100) {
            // abort on any errors.
            if (write_av_packet(opaque, &opaque->am_pkt) != PLAYER_SUCCESS)
                break;

            if (opaque->am_pkt.isvalid) {
                ALOGD("write_av_packet looping");
                loop++;
            }

            assert(loop < 100);
        }

        if(loop == 100) {
          ALOGD("amcodec stuck, reset!");
          reset(opaque);
        }

        d->pkt_temp.dts = AV_NOPTS_VALUE;
        d->pkt_temp.pts = AV_NOPTS_VALUE;

        d->packet_pending = 0;

        if (!d->pkt_temp.data) {
            ALOGD("finished");
            d->finished = d->pkt_serial;
        }
    }

fail:
    return ret;
}

static int reset(IJKAM_Pipenode_Opaque *opaque ) {

    ALOGD("amcodec reset!");

    am_codec_reset(&opaque->vcodec);

    int ret1 = am_codec_resume(&opaque->vcodec);
    int ret2 = am_codec_set_cntl_mode(&opaque->vcodec, TRICKMODE_NONE);

    am_packet_release(&opaque->am_pkt);
    am_packet_init(&opaque->am_pkt, &opaque->vcodec);
    int ret3 = pre_header_feeding(opaque, &opaque->am_pkt);

    opaque->decode_dts = 0;
    opaque->decode_pts = 0;
    opaque->do_discontinuity_check = 0;

    // return the first error.
    if(ret1 < 0)
      return ret1;
    if(ret2 < 0)
      return ret2;
    if(ret3 < 0)
      return ret3;

    return 0;
}

static void func_destroy(IJKFF_Pipenode *node)
{
    if (!node || !node->opaque)
        return;

    IJKAM_Pipenode_Opaque *opaque = node->opaque;

    // never leave vcodec ff/rw or paused.
    am_codec_resume(&opaque->vcodec);
    am_codec_set_cntl_mode(&opaque->vcodec, TRICKMODE_NONE);
    am_codec_close(&opaque->vcodec);

    am_packet_release(&opaque->am_pkt);
    free(opaque->extradata);
    opaque->extradata = NULL;

    // return tsync to default so external apps work
    sysfs_setint("/sys/class/tsync/enable", 1);
}

static int func_flush(IJKFF_Pipenode *node)
{
    return 0;
}

//
// This associates a PTS with the next input packet.
//
int check_in_pts(am_private_t *para, am_packet_t *pkt)
{
    int last_duration = 0;
    static int last_v_duration = 0;
    int64_t pts = 0;

    last_duration = last_v_duration;

    if (para->stream_type == AM_STREAM_ES) {
        if (INT64_0 != pkt->avpts) {
            pts = pkt->avpts;

            if (am_codec_checkin_pts(pkt->codec, pts) != 0) {
                ALOGE("ERROR check in pts error!");
                return PLAYER_PTS_ERROR;
            }

        } else if (INT64_0 != pkt->avdts) {
            pts = pkt->avdts * last_duration;

            if (am_codec_checkin_pts(pkt->codec, pts) != 0) {
                ALOGE("ERROR check in dts error!");
                return PLAYER_PTS_ERROR;
            }

            last_v_duration = pkt->avduration ? pkt->avduration : 1;
        } else {
            if (!para->check_first_pts) {
                if (am_codec_checkin_pts(pkt->codec, 0) != 0) {
                    ALOGE("ERROR check in 0 to video pts error!");
                    return PLAYER_PTS_ERROR;
                }
            }
        }

        para->check_first_pts = 1;
    }
    if (pts > 0)
      pkt->lastpts = pts;

    return PLAYER_SUCCESS;
}



static int func_run_sync(IJKFF_Pipenode *node)
{
    JNIEnv                *env      = NULL;
    IJKAM_Pipenode_Opaque *opaque   = node->opaque;
    FFPlayer              *ffp      = opaque->ffp;
    VideoState            *is       = ffp->is;
    Decoder               *d        = &is->viddec;
    PacketQueue           *q        = d->queue;
    int                    ret      = 0;
    int                    dequeue_count = 0;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed\n", __func__);
        return -1;
    }

    while (!q->abort_request) {
        //
        // Send a packet (an access unit) to the decoder
        //
        ret = feed_input_buffer(env, node, &dequeue_count);
        if (ret != 0) {
            goto fail;
        }

        //
        // If we couldn't read a packet, it's probably because we're paused or at
        // the end of the stream, so sleep for a bit and resume.
        //
        if(dequeue_count == 0 || is->paused) {
            usleep(1000000/100);
            //ALOGD("no dequeue");
            continue;
        }

        //
        // Synchronize the video to the master clock
        //
        syncToMaster(opaque);
    }
fail:
    return -1;
}

static void syncToMaster(IJKAM_Pipenode_Opaque *opaque)
{
    FFPlayer     *ffp = opaque->ffp;
    VideoState   *is  = ffp->is;
    Decoder      *d   = &is->viddec;

    //
    //
    // We have our amlogic video running. It presents frames (that we can't access) according
    // to pts values. It uses its own clock, pcrscr, that we can read and modify.
    //
    // ijkplayer is then presenting audio according to its own audio clock, which has some pts
    // time associated with that too.
    //
    // is->audclk gives the audio pts at the last packet played (or as close as possible).
    //
    // We adjust pcrscr so that it matches audclk, but we don't want to do that instantaneously
    // every frame because then the video jitters.
    //
    //

    int64_t pts_video = get_pts_video();    // pts of frame that was last displayed...?

    int64_t last_pts = opaque->decode_pts;

    // prevent decoder/demux from getting too far ahead of video
    int slept = 0;
    while(last_pts > 0 && (am_time_sub(last_pts, pts_video) > 90000*2)) {
        usleep(1000000/100);
        slept += 1;
        if(slept >= 100) {
            ALOGD("slept:%d pts_video:%jd decode_dts:%jd", slept, pts_video, last_pts);
            slept = 0;
            break;
        }
        pts_video = get_pts_video();
    }

    int64_t pts_audio = is->audclk.pts * 90000.0;

    // since audclk.pts was recorded time has advanced so take that into account.
    int64_t offset = av_gettime_relative() - ffp->audio_callback_time;
    offset = offset * 9/100;    // convert 1000KHz counter to 90KHz
    pts_audio += offset;

    pts_audio -= 10000;    // Magic!

    int64_t pts_pcrscr = get_pts_pcrscr();  // think this the master clock time for amcodec output

    int64_t delta = am_time_sub(pts_audio, pts_pcrscr);

    // modify pcrscr so that the frame presentation times are adjusted 'instantly'
    if(is->audclk.serial == d->pkt_serial) {
        if(abs(delta) > 5000) {
            set_pts_pcrscr(pts_pcrscr + delta);
            ALOGD("set pcr %jd!", pts_pcrscr + delta);
        }

        opaque->do_discontinuity_check = 1;
    }

#if TIMING_DEBUG
    ALOGD("pts_video:%lld pts_audio:%lld pts_pcrscr:%lld delta:%lld offset:%lld last_pts:%lld slept:%d sync_master:%d",
        pts_video, pts_audio, pts_pcrscr, delta, offset, last_pts, slept, get_master_sync_type(is));
#endif
}


static int write_header(am_private_t *para, am_packet_t *pkt)
{
    int write_bytes = 0, len = 0;

    if (pkt->hdr && pkt->hdr->size > 0) {
        if ((NULL == pkt->codec) || (NULL == pkt->hdr->data)) {
            ALOGE("[write_header]codec null!");
            return PLAYER_EMPTY_P;
        }
        //some wvc1 es data not need to add header
        if (para->video_format == VFORMAT_VC1 && para->video_codec_type == VIDEO_DEC_FORMAT_WVC1) {
            if ((pkt->data) && (pkt->data_size >= 4)
              && (pkt->data[0] == 0) && (pkt->data[1] == 0)
              && (pkt->data[2] == 1) && (pkt->data[3] == 0xd || pkt->data[3] == 0xf)) {
                return PLAYER_SUCCESS;
            }
        }
        while (1) {
            write_bytes = am_codec_write(pkt->codec, pkt->hdr->data + len, pkt->hdr->size - len);
            if (write_bytes < 0 || write_bytes > (pkt->hdr->size - len)) {
                if (-errno != AVERROR(EAGAIN)) {
                    ALOGE("ERROR:write header failed!");
                    return PLAYER_WR_FAILED;
                } else {
                    continue;
                }
            } else {
                //dumpfile_write(para, pkt->hdr->data, write_bytes);
                len += write_bytes;
                if (len == pkt->hdr->size) {
                    break;
                }
            }
        }
    }
    return PLAYER_SUCCESS;
}


static int write_av_packet(am_private_t *para, am_packet_t *pkt)
{
    int write_bytes = 0, len = 0;
    unsigned char *buf;
    int size;

    // do we need to check in pts or write the header ?
    if (pkt->newflag) {
        if (pkt->isvalid) {
            int ret = check_in_pts(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                ALOGE("check in pts failed");
                return PLAYER_WR_FAILED;
            }
        }
        if (write_header(para, pkt) == PLAYER_WR_FAILED) {
            ALOGE("[%s]write header failed!", __FUNCTION__);
            return PLAYER_WR_FAILED;
        }
        pkt->newflag = 0;
    }
  
    buf = pkt->data;
    size = pkt->data_size ;
    if (size == 0 && pkt->isvalid) {
        pkt->isvalid = 0;
        pkt->data_size = 0;
    }

    while (size > 0 && pkt->isvalid) {
        write_bytes = am_codec_write(pkt->codec, buf, size);
        if (write_bytes < 0 || write_bytes > size) {
            ALOGE("write codec data failed, write_bytes(%d), errno(%d), size(%d)", write_bytes, errno, size);
            if (-errno != AVERROR(EAGAIN)) {
                ALOGE("write codec data failed!");
                return PLAYER_WR_FAILED;
            } else {
                // adjust for any data we already wrote into codec.
                // we sleep a bit then exit as we will get called again
                // with the same pkt because pkt->isvalid has not been cleared.
                pkt->data += len;
                pkt->data_size -= len;
                usleep(RW_WAIT_TIME);
                ALOGE( "usleep(RW_WAIT_TIME), len(%d)", len);
                return PLAYER_SUCCESS;
            }
        } else {
            //dumpfile_write(para, buf, write_bytes);
            // keep track of what we write into codec from this pkt
            // in case we get hit with EAGAIN.
            len += write_bytes;
            if (len == pkt->data_size) {
                pkt->isvalid = 0;
                pkt->data_size = 0;
                break;
            } else if (len < pkt->data_size) {
                buf += write_bytes;
                size -= write_bytes;
            } else {
                // writing more that we should is a failure.
                return PLAYER_WR_FAILED;
            }
        }
    }

    return PLAYER_SUCCESS;
}

/*************************************************************************/
static int m4s2_dx50_mp4v_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
    if (size > pkt->hdr->size) {
        free(pkt->hdr->data), pkt->hdr->data = NULL;
        pkt->hdr->size = 0;

        pkt->hdr->data = (char*)malloc(size);
        if (!pkt->hdr->data) {
            ALOGD("[m4s2_dx50_add_header] NOMEM!");
            return PLAYER_FAILED;
        }
    }

    pkt->hdr->size = size;
    memcpy(pkt->hdr->data, buf, size);

    return PLAYER_SUCCESS;
}

static int m4s2_dx50_mp4v_write_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("m4s2_dx50_mp4v_write_header");
    int ret = m4s2_dx50_mp4v_add_header(para->extradata, para->extrasize, pkt);
    if (ret == PLAYER_SUCCESS) {
        if (1) {
            pkt->codec = &para->vcodec;
        } else {
            ALOGE("[m4s2_dx50_mp4v_add_header]invalid video codec!");
            return PLAYER_EMPTY_P;
        }
        pkt->newflag = 1;
        ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int mjpeg_data_prefeeding(am_packet_t *pkt)
{
    const unsigned char mjpeg_addon_data[] = {
        0xff, 0xd8, 0xff, 0xc4, 0x01, 0xa2, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x01, 0x00, 0x03, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x10,
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
        0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
        0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1,
        0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72,
        0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29,
        0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64,
        0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
        0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
        0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4,
        0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8,
        0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
        0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0x11, 0x00, 0x02, 0x01,
        0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51,
        0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1,
        0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24,
        0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a,
        0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
        0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82,
        0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
        0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
        0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4,
        0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa
    };

    if (pkt->hdr->data) {
        memcpy(pkt->hdr->data, &mjpeg_addon_data, sizeof(mjpeg_addon_data));
        pkt->hdr->size = sizeof(mjpeg_addon_data);
    } else {
        ALOGE("[mjpeg_data_prefeeding]No enough memory!");
        return PLAYER_FAILED;
    }
    return PLAYER_SUCCESS;
}

static int mjpeg_write_header(am_private_t *para, am_packet_t *pkt)
{
    mjpeg_data_prefeeding(pkt);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        ALOGE("[mjpeg_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    write_av_packet(para, pkt);
    return PLAYER_SUCCESS;
}

static int divx3_data_prefeeding(am_packet_t *pkt, unsigned w, unsigned h)
{
    unsigned i = (w << 12) | (h & 0xfff);
    unsigned char divx311_add[10] = {
        0x00, 0x00, 0x00, 0x01,
        0x20, 0x00, 0x00, 0x00,
        0x00, 0x00
    };
    divx311_add[5] = (i >> 16) & 0xff;
    divx311_add[6] = (i >> 8) & 0xff;
    divx311_add[7] = i & 0xff;

    if (pkt->hdr->data) {
        memcpy(pkt->hdr->data, divx311_add, sizeof(divx311_add));
        pkt->hdr->size = sizeof(divx311_add);
    } else {
        ALOGE("[divx3_data_prefeeding]No enough memory!");
        return PLAYER_FAILED;
    }
    return PLAYER_SUCCESS;
}

static int divx3_write_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("divx3_write_header");
    divx3_data_prefeeding(pkt, para->video_width, para->video_height);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        ALOGE("[divx3_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    write_av_packet(para, pkt);
    return PLAYER_SUCCESS;
}

static int h264_add_header(unsigned char *buf, int size, am_packet_t *pkt)
{
    if (size > HDR_BUF_SIZE)
    {
        free(pkt->hdr->data);
        pkt->hdr->data = (char *)malloc(size);
        if (!pkt->hdr->data)
            return PLAYER_NOMEM;
    }

    memcpy(pkt->hdr->data, buf, size);
    pkt->hdr->size = size;
    return PLAYER_SUCCESS;
}

static int h264_write_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("h264_write_header");
    int ret = h264_add_header(para->extradata, para->extrasize, pkt);
    if (ret == PLAYER_SUCCESS) {
        //if (ctx->vcodec) {
        if (1) {
            pkt->codec = &para->vcodec;
        } else {
            ALOGE("[pre_header_feeding]invalid video codec!");
            return PLAYER_EMPTY_P;
        }

        pkt->newflag = 1;
        ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int hevc_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
    if (size > HDR_BUF_SIZE)
    {
        free(pkt->hdr->data);
        pkt->hdr->data = (char *)malloc(size);
        if (!pkt->hdr->data)
            return PLAYER_NOMEM;
    }

    memcpy(pkt->hdr->data, buf, size);
    pkt->hdr->size = size;
    return PLAYER_SUCCESS;
}

static int hevc_write_header(am_private_t *para, am_packet_t *pkt)
{
    int ret = -1;

    if (para->extradata) {
      ret = hevc_add_header(para->extradata, para->extrasize, pkt);
    }
    if (ret == PLAYER_SUCCESS) {
      pkt->codec = &para->vcodec;
      pkt->newflag = 1;
      ret = write_av_packet(para, pkt);
    }
    return ret;
}

static int wmv3_write_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("wmv3_write_header");
    unsigned i, check_sum = 0;
    unsigned data_len = para->extrasize + 4;

    pkt->hdr->data[0] = 0;
    pkt->hdr->data[1] = 0;
    pkt->hdr->data[2] = 1;
    pkt->hdr->data[3] = 0x10;

    pkt->hdr->data[4] = 0;
    pkt->hdr->data[5] = (data_len >> 16) & 0xff;
    pkt->hdr->data[6] = 0x88;
    pkt->hdr->data[7] = (data_len >> 8) & 0xff;
    pkt->hdr->data[8] = data_len & 0xff;
    pkt->hdr->data[9] = 0x88;

    pkt->hdr->data[10] = 0xff;
    pkt->hdr->data[11] = 0xff;
    pkt->hdr->data[12] = 0x88;
    pkt->hdr->data[13] = 0xff;
    pkt->hdr->data[14] = 0xff;
    pkt->hdr->data[15] = 0x88;

    for (i = 4 ; i < 16 ; i++) {
        check_sum += pkt->hdr->data[i];
    }

    pkt->hdr->data[16] = (check_sum >> 8) & 0xff;
    pkt->hdr->data[17] =  check_sum & 0xff;
    pkt->hdr->data[18] = 0x88;
    pkt->hdr->data[19] = (check_sum >> 8) & 0xff;
    pkt->hdr->data[20] =  check_sum & 0xff;
    pkt->hdr->data[21] = 0x88;

    pkt->hdr->data[22] = (para->video_width >> 8) & 0xff;
    pkt->hdr->data[23] =  para->video_width & 0xff;
    pkt->hdr->data[24] = (para->video_height >> 8) & 0xff;
    pkt->hdr->data[25] =  para->video_height & 0xff;

    memcpy(pkt->hdr->data + 26, para->extradata, para->extrasize);
    pkt->hdr->size = para->extrasize + 26;
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        ALOGE("[wmv3_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

static int wvc1_write_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("wvc1_write_header");
    memcpy(pkt->hdr->data, para->extradata + 1, para->extrasize - 1);
    pkt->hdr->size = para->extrasize - 1;
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        ALOGE("[wvc1_write_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }
    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

static int mpeg_add_header(am_private_t *para, am_packet_t *pkt)
{
    ALOGD("mpeg_add_header");
#define STUFF_BYTES_LENGTH     (256)
    int size;
    unsigned char packet_wrapper[] = {
        0x00, 0x00, 0x01, 0xe0,
        0x00, 0x00,                                /* pes packet length */
        0x81, 0xc0, 0x0d,
        0x20, 0x00, 0x00, 0x00, 0x00, /* PTS */
        0x1f, 0xff, 0xff, 0xff, 0xff, /* DTS */
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    size = para->extrasize + sizeof(packet_wrapper);
    packet_wrapper[4] = size >> 8 ;
    packet_wrapper[5] = size & 0xff ;
    memcpy(pkt->hdr->data, packet_wrapper, sizeof(packet_wrapper));
    size = sizeof(packet_wrapper);
    //ALOGD("[mpeg_add_header:%d]wrapper size=%d\n",__LINE__,size);
    memcpy(pkt->hdr->data + size, para->extradata, para->extrasize);
    size += para->extrasize;
    //ALOGD("[mpeg_add_header:%d]wrapper+seq size=%d\n",__LINE__,size);
    memset(pkt->hdr->data + size, 0xff, STUFF_BYTES_LENGTH);
    size += STUFF_BYTES_LENGTH;
    pkt->hdr->size = size;
    //ALOGD("[mpeg_add_header:%d]hdr_size=%d\n",__LINE__,size);
    if (1) {
        pkt->codec = &para->vcodec;
    } else {
        ALOGD("[mpeg_add_header]invalid codec!");
        return PLAYER_EMPTY_P;
    }

    pkt->newflag = 1;
    return write_av_packet(para, pkt);
}

static int pre_header_feeding(am_private_t *para, am_packet_t *pkt)
{
    int ret;
    if (para->stream_type == AM_STREAM_ES) {
        if (pkt->hdr == NULL) {
            pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
            pkt->hdr->data = (char *)malloc(HDR_BUF_SIZE);
            if (!pkt->hdr->data) {
                //ALOGE("[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
        }

        if (VFORMAT_H264 == para->video_format || VFORMAT_H264_4K2K == para->video_format) {
            ret = h264_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((VFORMAT_MPEG4 == para->video_format) && (VIDEO_DEC_FORMAT_MPEG4_3 == para->video_codec_type)) {
            ret = divx3_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((CODEC_TAG_M4S2 == para->video_codec_tag)
                || (CODEC_TAG_DX50 == para->video_codec_tag)
                || (CODEC_TAG_mp4v == para->video_codec_tag)) {
            ret = m4s2_dx50_mp4v_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        /*
        } else if ((AVI_FILE == para->file_type)
                && (VIDEO_DEC_FORMAT_MPEG4_3 != para->vstream_info.video_codec_type)
                && (VFORMAT_H264 != para->vstream_info.video_format)
                && (VFORMAT_VC1 != para->vstream_info.video_format)) {
            ret = avi_write_header(para);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        */
        } else if (CODEC_TAG_WMV3 == para->video_codec_tag) {
            ALOGD("CODEC_TAG_WMV3 == para->video_codec_tag");
            ret = wmv3_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if ((CODEC_TAG_WVC1 == para->video_codec_tag)
                || (CODEC_TAG_VC_1 == para->video_codec_tag)
                || (CODEC_TAG_WMVA == para->video_codec_tag)) {
            ALOGD("CODEC_TAG_WVC1 == para->video_codec_tag");
            ret = wvc1_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        /*
        } else if ((MKV_FILE == para->file_type) &&
                  ((VFORMAT_MPEG4 == para->vstream_info.video_format)
                || (VFORMAT_MPEG12 == para->vstream_info.video_format))) {
            ret = mkv_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        */
        } else if (VFORMAT_MJPEG == para->video_format) {
            ret = mjpeg_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        } else if (VFORMAT_HEVC == para->video_format) {
            ret = hevc_write_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        }

        if (pkt->hdr) {
            if (pkt->hdr->data) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }
            free(pkt->hdr);
            pkt->hdr = NULL;
        }
    }
    else if (para->stream_type == AM_STREAM_PS) {
        if (pkt->hdr == NULL) {
            pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
            pkt->hdr->data = (char*)malloc(HDR_BUF_SIZE);
            if (!pkt->hdr->data) {
                ALOGD("[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
        }
        if (( AV_CODEC_ID_MPEG1VIDEO == para->video_codec_id)
          || (AV_CODEC_ID_MPEG2VIDEO == para->video_codec_id)
          || (AV_CODEC_ID_MPEG2VIDEO_XVMC == para->video_codec_id)) {
            ret = mpeg_add_header(para, pkt);
            if (ret != PLAYER_SUCCESS) {
                return ret;
            }
        }
        if (pkt->hdr) {
            if (pkt->hdr->data) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }
            free(pkt->hdr);
            pkt->hdr = NULL;
        }
    }
    return PLAYER_SUCCESS;
}



int divx3_prefix(am_packet_t *pkt)
{
#define DIVX311_CHUNK_HEAD_SIZE 13
    const unsigned char divx311_chunk_prefix[DIVX311_CHUNK_HEAD_SIZE] = {
        0x00, 0x00, 0x00, 0x01, 0xb6, 'D', 'I', 'V', 'X', '3', '.', '1', '1'
    };
    if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
        free(pkt->hdr->data);
        pkt->hdr->data = NULL;
    }

    if (pkt->hdr == NULL) {
        pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
        if (!pkt->hdr) {
            ALOGD("[divx3_prefix] NOMEM!");
            return PLAYER_FAILED;
        }

        pkt->hdr->data = NULL;
        pkt->hdr->size = 0;
    }

    pkt->hdr->data = (char*)malloc(DIVX311_CHUNK_HEAD_SIZE + 4);
    if (pkt->hdr->data == NULL) {
        ALOGD("[divx3_prefix] NOMEM!");
        return PLAYER_FAILED;
    }

    memcpy(pkt->hdr->data, divx311_chunk_prefix, DIVX311_CHUNK_HEAD_SIZE);

    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 0] = (pkt->data_size >> 24) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 1] = (pkt->data_size >> 16) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 2] = (pkt->data_size >>  8) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 3] = pkt->data_size & 0xff;

    pkt->hdr->size = DIVX311_CHUNK_HEAD_SIZE + 4;
    pkt->newflag = 1;

    return PLAYER_SUCCESS;
}


static int set_header_info(am_private_t *para)
{
  am_packet_t *pkt = &para->am_pkt;

  //if (pkt->newflag)
  {
    //if (pkt->hdr)
    //  pkt->hdr->size = 0;

    if (para->video_format == VFORMAT_MPEG4)
    {
      if (para->video_codec_type == VIDEO_DEC_FORMAT_MPEG4_3)
      {
        return divx3_prefix(pkt);
      }
      else if (para->video_codec_type == VIDEO_DEC_FORMAT_H263)
      {
        return PLAYER_UNSUPPORT;
      }
    } else if (para->video_format == VFORMAT_VC1) {
        if (para->video_codec_type == VIDEO_DEC_FORMAT_WMV3) {
            unsigned i, check_sum = 0, data_len = 0;

            if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }

            if (pkt->hdr == NULL) {
                pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
                if (!pkt->hdr) {
                    return PLAYER_FAILED;
                }

                pkt->hdr->data = NULL;
                pkt->hdr->size = 0;
            }

            if (pkt->avpkt.flags) {
                pkt->hdr->data = (char*)malloc(para->extrasize + 26 + 22);
                if (pkt->hdr->data == NULL) {
                    return PLAYER_FAILED;
                }

                pkt->hdr->data[0] = 0;
                pkt->hdr->data[1] = 0;
                pkt->hdr->data[2] = 1;
                pkt->hdr->data[3] = 0x10;

                data_len = para->extrasize + 4;
                pkt->hdr->data[4] = 0;
                pkt->hdr->data[5] = (data_len >> 16) & 0xff;
                pkt->hdr->data[6] = 0x88;
                pkt->hdr->data[7] = (data_len >> 8) & 0xff;
                pkt->hdr->data[8] =  data_len & 0xff;
                pkt->hdr->data[9] = 0x88;

                pkt->hdr->data[10] = 0xff;
                pkt->hdr->data[11] = 0xff;
                pkt->hdr->data[12] = 0x88;
                pkt->hdr->data[13] = 0xff;
                pkt->hdr->data[14] = 0xff;
                pkt->hdr->data[15] = 0x88;

                for (i = 4 ; i < 16 ; i++) {
                    check_sum += pkt->hdr->data[i];
                }

                pkt->hdr->data[16] = (check_sum >> 8) & 0xff;
                pkt->hdr->data[17] =  check_sum & 0xff;
                pkt->hdr->data[18] = 0x88;
                pkt->hdr->data[19] = (check_sum >> 8) & 0xff;
                pkt->hdr->data[20] =  check_sum & 0xff;
                pkt->hdr->data[21] = 0x88;

                pkt->hdr->data[22] = (para->video_width  >> 8) & 0xff;
                pkt->hdr->data[23] =  para->video_width  & 0xff;
                pkt->hdr->data[24] = (para->video_height >> 8) & 0xff;
                pkt->hdr->data[25] =  para->video_height & 0xff;

                memcpy(pkt->hdr->data + 26, para->extradata, para->extrasize);

                check_sum = 0;
                data_len = para->extrasize + 26;
            } else {
                pkt->hdr->data = (char*)malloc(22);
                if (pkt->hdr->data == NULL) {
                    return PLAYER_FAILED;
                }
            }

            pkt->hdr->data[data_len + 0]  = 0;
            pkt->hdr->data[data_len + 1]  = 0;
            pkt->hdr->data[data_len + 2]  = 1;
            pkt->hdr->data[data_len + 3]  = 0xd;

            pkt->hdr->data[data_len + 4]  = 0;
            pkt->hdr->data[data_len + 5]  = (pkt->data_size >> 16) & 0xff;
            pkt->hdr->data[data_len + 6]  = 0x88;
            pkt->hdr->data[data_len + 7]  = (pkt->data_size >> 8) & 0xff;
            pkt->hdr->data[data_len + 8]  =  pkt->data_size & 0xff;
            pkt->hdr->data[data_len + 9]  = 0x88;

            pkt->hdr->data[data_len + 10] = 0xff;
            pkt->hdr->data[data_len + 11] = 0xff;
            pkt->hdr->data[data_len + 12] = 0x88;
            pkt->hdr->data[data_len + 13] = 0xff;
            pkt->hdr->data[data_len + 14] = 0xff;
            pkt->hdr->data[data_len + 15] = 0x88;

            for (i = data_len + 4 ; i < data_len + 16 ; i++) {
                check_sum += pkt->hdr->data[i];
            }

            pkt->hdr->data[data_len + 16] = (check_sum >> 8) & 0xff;
            pkt->hdr->data[data_len + 17] =  check_sum & 0xff;
            pkt->hdr->data[data_len + 18] = 0x88;
            pkt->hdr->data[data_len + 19] = (check_sum >> 8) & 0xff;
            pkt->hdr->data[data_len + 20] =  check_sum & 0xff;
            pkt->hdr->data[data_len + 21] = 0x88;

            pkt->hdr->size = data_len + 22;
            pkt->newflag = 1;
        } else if (para->video_codec_type == VIDEO_DEC_FORMAT_WVC1) {
            if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL)) {
                free(pkt->hdr->data);
                pkt->hdr->data = NULL;
            }

            if (pkt->hdr == NULL) {
                pkt->hdr = (hdr_buf_t*)malloc(sizeof(hdr_buf_t));
                if (!pkt->hdr) {
                    ALOGD("[wvc1_prefix] NOMEM!");
                    return PLAYER_FAILED;
                }

                pkt->hdr->data = NULL;
                pkt->hdr->size = 0;
            }

            pkt->hdr->data = (char*)malloc(4);
            if (pkt->hdr->data == NULL) {
                ALOGD("[wvc1_prefix] NOMEM!");
                return PLAYER_FAILED;
            }

            pkt->hdr->data[0] = 0;
            pkt->hdr->data[1] = 0;
            pkt->hdr->data[2] = 1;
            pkt->hdr->data[3] = 0xd;
            pkt->hdr->size = 4;
            pkt->newflag = 1;
        }
    }
  }
  return PLAYER_SUCCESS;
}
