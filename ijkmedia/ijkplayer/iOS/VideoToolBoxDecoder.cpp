//
//  VideoToolBoxDecoder.cpp
//  MediaPlayer
//
//  Created by 施灵凯 on 15/4/15.
//  Copyright (c) 2015年 bolome. All rights reserved.
//

#include "VideoToolBoxDecoder.h"
#include "Media_Log.h"

#include "iOSUtils.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
}

#include <pthread.h>

#if defined(__cplusplus)
extern "C"
{
#endif
    
#pragma pack(push, 4)
    
    //-----------------------------------------------------------------------------------
    // /System/Library/PrivateFrameworks/VideoToolbox.framework
    enum VTFormat {
        kVTFormatJPEG         = 'jpeg', // kCMVideoCodecType_JPEG
        kVTFormatH264         = 'avc1', // kCMVideoCodecType_H264 (MPEG-4 Part 10))
        kVTFormatMPEG4Video   = 'mp4v', // kCMVideoCodecType_MPEG4Video (MPEG-4 Part 2)
        kVTFormatMPEG2Video   = 'mp2v'  // kCMVideoCodecType_MPEG2Video
    };
    enum {
        kVTDecoderNoErr = 0,
        kVTDecoderHardwareNotSupportedErr = -12470,
        kVTDecoderFormatNotSupportedErr = -12471,
        kVTDecoderConfigurationError = -12472,
        kVTDecoderDecoderFailedErr = -12473,
    };
    enum {
        kVTDecodeInfo_Asynchronous = 1UL << 0,
        kVTDecodeInfo_FrameDropped = 1UL << 1
    };
    enum {
        // tells the decoder not to bother returning a CVPixelBuffer
        // in the outputCallback. The output callback will still be called.
        kVTDecoderDecodeFlags_DontEmitFrame = 1 << 1,
    };
    enum {
        // decode and return buffers for all frames currently in flight.
        kVTDecoderFlush_EmitFrames = 1 << 0
    };
    
    typedef UInt32 VTFormatId;
    typedef CFTypeRef VTDecompressionSessionRef;
    
    typedef void (*VTDecompressionOutputCallbackFunc)(
    void            *refCon,
    CFDictionaryRef frameInfo,
    OSStatus        status,
    UInt32          infoFlags,
    CVBufferRef     imageBuffer);
    
    typedef struct _VTDecompressionOutputCallback VTDecompressionOutputCallback;
    struct _VTDecompressionOutputCallback {
        VTDecompressionOutputCallbackFunc callback;
        void *refcon;
    };
    
    extern CFStringRef kVTVideoDecoderSpecification_EnableSandboxedVideoDecoder;
    
    extern OSStatus VTDecompressionSessionCreate(
                                                 CFAllocatorRef allocator,
                                                 CMFormatDescriptionRef videoFormatDescription,
                                                 CFTypeRef sessionOptions,
                                                 CFDictionaryRef destinationPixelBufferAttributes,
                                                 VTDecompressionOutputCallback *outputCallback,
                                                 VTDecompressionSessionRef *session);
    
    extern OSStatus VTDecompressionSessionDecodeFrame(
                                                      VTDecompressionSessionRef session, CMSampleBufferRef sbuf,
                                                      uint32_t decoderFlags, CFDictionaryRef frameInfo, uint32_t unk1);
    
    extern OSStatus VTDecompressionSessionCopyProperty(VTDecompressionSessionRef session, CFTypeRef key, void* unk, CFTypeRef * value);
    extern OSStatus VTDecompressionSessionCopySupportedPropertyDictionary(VTDecompressionSessionRef session, CFDictionaryRef * dict);
    extern OSStatus VTDecompressionSessionSetProperty(VTDecompressionSessionRef session, CFStringRef propName, CFTypeRef propValue);
    extern void VTDecompressionSessionInvalidate(VTDecompressionSessionRef session);
    extern void VTDecompressionSessionRelease(VTDecompressionSessionRef session);
    extern VTDecompressionSessionRef VTDecompressionSessionRetain(VTDecompressionSessionRef session);
    extern OSStatus VTDecompressionSessionWaitForAsynchronousFrames(VTDecompressionSessionRef session);
    
    //-----------------------------------------------------------------------------------
    // /System/Library/Frameworks/CoreMedia.framework
    union
    {
        void* lpAddress;
        // iOS <= 4.2
        OSStatus (*FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom1)(
                                                                                       CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
                                                                                       UInt32 atomId, const UInt8 *data, CFIndex len, CMFormatDescriptionRef *formatDesc);
        // iOS >= 4.3
        OSStatus (*FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom2)(
                                                                                       CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
                                                                                       UInt32 atomId, const UInt8 *data, CFIndex len, CFDictionaryRef extensions, CMFormatDescriptionRef *formatDesc);
    } FigVideoHack;
    extern OSStatus FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom(
                                                                                      CFAllocatorRef allocator, UInt32 formatId, UInt32 width, UInt32 height,
                                                                                      UInt32 atomId, const UInt8 *data, CFIndex len, CMFormatDescriptionRef *formatDesc);
    
    extern CMSampleBufferRef FigSampleBufferRetain(CMSampleBufferRef buf);
    //-----------------------------------------------------------------------------------
#pragma pack(pop)
    
#if defined(__cplusplus)
}
#endif

int CheckNP2( unsigned x )
{
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return ++x;
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// helper functions for debuging VTDecompression
#if _DEBUG
char* vtutil_string_to_utf8(CFStringRef s)
{
    char *result;
    CFIndex size;
    
    size = CFStringGetMaximumSizeForEncoding(CFStringGetLength (s), kCFStringEncodingUTF8);
    result = (char*)malloc(size + 1);
    CFStringGetCString(s, result, size + 1, kCFStringEncodingUTF8);
    
    return result;
}

char* vtutil_object_to_string(CFTypeRef obj)
{
    char *result;
    CFStringRef s;
    
    if (obj == NULL)
        return strdup ("(null)");
    
    s = CFCopyDescription(obj);
    result = vtutil_string_to_utf8(s);
    CFRelease(s);
    
    return result;
}

typedef struct {
    VTDecompressionSessionRef session;
} VTDumpDecompressionPropCtx;

void
vtdec_session_dump_property(CFStringRef prop_name, CFDictionaryRef prop_attrs, VTDumpDecompressionPropCtx *dpc)
{
    char *name_str;
    CFTypeRef prop_value;
    OSStatus status;
    
    name_str = vtutil_string_to_utf8(prop_name);
    if (true)
    {
        char *attrs_str;
        
        attrs_str = vtutil_object_to_string(prop_attrs);
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
            CLog::Log(LOGDEBUG, "%s = %s\n", name_str, attrs_str);
        free(attrs_str);
    }
    
    status = VTDecompressionSessionCopyProperty(dpc->session, prop_name, NULL, &prop_value);
    if (status == kVTDecoderNoErr)
    {
        char *value_str;
        
        value_str = vtutil_object_to_string(prop_value);
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
            CLog::Log(LOGDEBUG, "%s = %s\n", name_str, value_str);
        free(value_str);
        
        if (prop_value != NULL)
            CFRelease(prop_value);
    }
    else
    {
        if (g_advancedSettings.CanLogComponent(LOGVIDEO))
            CLog::Log(LOGDEBUG, "%s = <failed to query: %d>\n", name_str, (int)status);
    }
    
    free(name_str);
}

void vtdec_session_dump_properties(VTDecompressionSessionRef session)
{
    VTDumpDecompressionPropCtx dpc = { session };
    CFDictionaryRef dict;
    OSStatus status;
    
    status = VTDecompressionSessionCopySupportedPropertyDictionary(session, &dict);
    if (status != kVTDecoderNoErr)
        goto error;
    CFDictionaryApplyFunction(dict, (CFDictionaryApplierFunction)vtdec_session_dump_property, &dpc);
    CFRelease(dict);
    
    return;
    
error:
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
        CLog::Log(LOGDEBUG, "failed to dump properties\n");
}
#endif
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// helper function that inserts an int32_t into a dictionary
static void
CFDictionarySetSInt32(CFMutableDictionaryRef dictionary, CFStringRef key, SInt32 numberSInt32)
{
    CFNumberRef number;
    
    number = CFNumberCreate(NULL, kCFNumberSInt32Type, &numberSInt32);
    CFDictionarySetValue(dictionary, key, number);
    CFRelease(number);
}
// helper function that inserts an double into a dictionary
static void
CFDictionarySetDouble(CFMutableDictionaryRef dictionary, CFStringRef key, double numberDouble)
{
    CFNumberRef number;
    
    number = CFNumberCreate(NULL, kCFNumberDoubleType, &numberDouble);
    CFDictionaryAddValue(dictionary, key, number);
    CFRelease(number);
}
// helper function that wraps dts/pts into a dictionary
static CFDictionaryRef
CreateDictionaryWithDisplayTime(double time, double dts, double pts)
{
    CFStringRef key[3] = {
        CFSTR("VideoDisplay_TIME"),
        CFSTR("VideoDisplay_DTS"),
        CFSTR("VideoDisplay_PTS")};
    CFNumberRef value[3];
    CFDictionaryRef display_time;
    
    value[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &time);
    value[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &dts);
    value[2] = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &pts);
    
    display_time = CFDictionaryCreate(
                                      kCFAllocatorDefault, (const void **)&key, (const void **)&value, 3,
                                      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFRelease(value[0]);
    CFRelease(value[1]);
    CFRelease(value[2]);
    
    return display_time;
}
// helper function to extract dts/pts from a dictionary
static void
GetFrameDisplayTimeFromDictionary(
                                  CFDictionaryRef inFrameInfoDictionary, frame_queue *frame)
{
    // default to AV_NOPTS_VALUE
    frame->sort_time = -1.0;
    frame->dts = AV_NOPTS_VALUE;
    frame->pts = AV_NOPTS_VALUE;
    if (inFrameInfoDictionary == NULL)
        return;
    
    CFNumberRef value[3];
    //
    value[0] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_TIME"));
    if (value[0])
        CFNumberGetValue(value[0], kCFNumberDoubleType, &frame->sort_time);
    value[1] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_DTS"));
    if (value[1])
        CFNumberGetValue(value[1], kCFNumberDoubleType, &frame->dts);
    value[2] = (CFNumberRef)CFDictionaryGetValue(inFrameInfoDictionary, CFSTR("VideoDisplay_PTS"));
    if (value[2])
        CFNumberGetValue(value[2], kCFNumberDoubleType, &frame->pts);
    
    return;
}
// helper function to create a format descriptor
static CMFormatDescriptionRef
CreateFormatDescription(VTFormatId format_id, int width, int height)
{
    CMFormatDescriptionRef fmt_desc;
    OSStatus status;
    
    status = CMVideoFormatDescriptionCreate(
                                            NULL,             // CFAllocatorRef allocator
                                            format_id,
                                            width,
                                            height,
                                            NULL,             // CFDictionaryRef extensions
                                            &fmt_desc);
    
    if (status == kVTDecoderNoErr)
        return fmt_desc;
    else
        return NULL;
}
// helper function to create a avcC atom format descriptor
static CMFormatDescriptionRef
CreateFormatDescriptionFromCodecData(VTFormatId format_id, int width, int height, const uint8_t *extradata, int extradata_size, uint32_t atom)
{
    CMFormatDescriptionRef fmt_desc = NULL;
    OSStatus status;
    
    FigVideoHack.lpAddress = (void*)FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom;
    
    if (iOSUtils::GetIOSVersion() < 4.3)
    {
        LOGD("%s - GetIOSVersion says < 4.3", __FUNCTION__);
        status = FigVideoHack.FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom1(
                                                                                                 NULL,
                                                                                                 format_id,
                                                                                                 width,
                                                                                                 height,
                                                                                                 atom,
                                                                                                 extradata,
                                                                                                 extradata_size,
                                                                                                 &fmt_desc);
    }
    else
    {
        LOGD("%s - GetIOSVersion says >= 4.3", __FUNCTION__);
        status = FigVideoHack.FigVideoFormatDescriptionCreateWithSampleDescriptionExtensionAtom2(
                                                                                                 NULL,
                                                                                                 format_id,
                                                                                                 width,
                                                                                                 height,
                                                                                                 atom,
                                                                                                 extradata,
                                                                                                 extradata_size,
                                                                                                 NULL,
                                                                                                 &fmt_desc);
    }
    
    if (status == kVTDecoderNoErr)
        return fmt_desc;
    else
        return NULL;
}
// helper function to create a CMSampleBufferRef from demuxer data
static CMSampleBufferRef
CreateSampleBufferFrom(CMFormatDescriptionRef fmt_desc, void *demux_buff, size_t demux_size)
{
    OSStatus status;
    CMBlockBufferRef newBBufOut = NULL;
    CMSampleBufferRef sBufOut = NULL;
    
    status = CMBlockBufferCreateWithMemoryBlock(
                                                NULL,             // CFAllocatorRef structureAllocator
                                                demux_buff,       // void *memoryBlock
                                                demux_size,       // size_t blockLengt
                                                kCFAllocatorNull, // CFAllocatorRef blockAllocator
                                                NULL,             // const CMBlockBufferCustomBlockSource *customBlockSource
                                                0,                // size_t offsetToData
                                                demux_size,       // size_t dataLength
                                                FALSE,            // CMBlockBufferFlags flags
                                                &newBBufOut);     // CMBlockBufferRef *newBBufOut
    
    if (!status)
    {
        status = CMSampleBufferCreate(
                                      NULL,           // CFAllocatorRef allocator
                                      newBBufOut,     // CMBlockBufferRef dataBuffer
                                      TRUE,           // Boolean dataReady
                                      0,              // CMSampleBufferMakeDataReadyCallback makeDataReadyCallback
                                      0,              // void *makeDataReadyRefcon
                                      fmt_desc,       // CMFormatDescriptionRef formatDescription
                                      1,              // CMItemCount numSamples
                                      0,              // CMItemCount numSampleTimingEntries
                                      NULL,           // const CMSampleTimingInfo *sampleTimingArray
                                      0,              // CMItemCount numSampleSizeEntries
                                      NULL,           // const size_t *sampleSizeArray
                                      &sBufOut);      // CMSampleBufferRef *sBufOut
    }
    
    CFRelease(newBBufOut);
    
    /*
     CLog::Log(LOGDEBUG, "%s - CreateSampleBufferFrom size %ld demux_buff [0x%08x] sBufOut [0x%08x]",
     __FUNCTION__, demux_size, (unsigned int)demux_buff, (unsigned int)sBufOut);
     */
    
    return sBufOut;
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
/* MPEG-4 esds (elementary stream descriptor) */
typedef struct {
    int version;
    long flags;
    
    uint16_t esid;
    uint8_t  stream_priority;
    
    uint8_t  objectTypeId;
    uint8_t  streamType;
    uint32_t bufferSizeDB;
    uint32_t maxBitrate;
    uint32_t avgBitrate;
    
    int      decoderConfigLen;
    uint8_t* decoderConfig;
} quicktime_esds_t;

int quicktime_write_mp4_descr_length(AVIOContext *pb, int length, int compact)
{
    int i;
    uint8_t b;
    int numBytes;
    
    if (compact)
    {
        if (length <= 0x7F)
        {
            numBytes = 1;
        }
        else if (length <= 0x3FFF)
        {
            numBytes = 2;
        }
        else if (length <= 0x1FFFFF)
        {
            numBytes = 3;
        }
        else
        {
            numBytes = 4;
        }
    }
    else
    {
        numBytes = 4;
    }
    
    for (i = numBytes-1; i >= 0; i--)
    {
        b = (length >> (i * 7)) & 0x7F;
        if (i != 0)
        {
            b |= 0x80;
        }
        avio_w8(pb, b);
    }
    
    return numBytes;
}

void quicktime_write_esds(AVIOContext *pb, quicktime_esds_t *esds)
{
    avio_w8(pb, 0);     // Version
    avio_wb24(pb, 0);     // Flags
    
    // elementary stream descriptor tag
    avio_w8(pb, 0x03);
    quicktime_write_mp4_descr_length(pb,
                                     3 + 5 + (13 + 5 + esds->decoderConfigLen) + 3, false);
    // 3 bytes + 5 bytes for tag
    avio_wb16(pb, esds->esid);
    avio_w8(pb, esds->stream_priority);
    
    // decoder configuration description tag
    avio_w8(pb, 0x04);
    quicktime_write_mp4_descr_length(pb,
                                     13 + 5 + esds->decoderConfigLen, false);
    // 13 bytes + 5 bytes for tag
    avio_w8(pb, esds->objectTypeId); // objectTypeIndication
    avio_w8(pb, esds->streamType);   // streamType
    avio_wb24(pb, esds->bufferSizeDB); // buffer size
    avio_wb32(pb, esds->maxBitrate);   // max bitrate
    avio_wb32(pb, esds->avgBitrate);   // average bitrate
    
    // decoder specific description tag
    avio_w8(pb, 0x05);
    quicktime_write_mp4_descr_length(pb, esds->decoderConfigLen, false);
    avio_write(pb, esds->decoderConfig, esds->decoderConfigLen);
    
    // sync layer configuration descriptor tag
    avio_w8(pb, 0x06);  // tag
    avio_w8(pb, 0x01);  // length
    avio_w8(pb, 0x7F);  // no SL
    
    /* no IPI_DescrPointer */
    /* no IP_IdentificationDataSet */
    /* no IPMP_DescriptorPointer */
    /* no LanguageDescriptor */
    /* no QoS_Descriptor */
    /* no RegistrationDescriptor */
    /* no ExtensionDescriptor */
    
}

quicktime_esds_t* quicktime_set_esds(const uint8_t * decoderConfig, int decoderConfigLen)
{
    // ffmpeg's codec->avctx->extradata, codec->avctx->extradata_size
    // are decoderConfig/decoderConfigLen
    quicktime_esds_t *esds;
    
    esds = (quicktime_esds_t*)malloc(sizeof(quicktime_esds_t));
    memset(esds, 0, sizeof(quicktime_esds_t));
    
    esds->version         = 0;
    esds->flags           = 0;
    
    esds->esid            = 0;
    esds->stream_priority = 0;      // 16 ? 0x1f
    
    esds->objectTypeId    = 32;     // 32 = AV_CODEC_ID_MPEG4, 33 = AV_CODEC_ID_H264
    // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
    // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
    esds->streamType      = 0x11;
    esds->bufferSizeDB    = 64000;  // Hopefully not important :)
    
    // Maybe correct these later?
    esds->maxBitrate      = 200000; // 0 for vbr
    esds->avgBitrate      = 200000;
    
    esds->decoderConfigLen = decoderConfigLen;
    esds->decoderConfig = (uint8_t*)malloc(esds->decoderConfigLen);
    memcpy(esds->decoderConfig, decoderConfig, esds->decoderConfigLen);
    return esds;
}

void quicktime_esds_dump(quicktime_esds_t * esds)
{
    int i;
    printf("esds: \n");
    printf(" Version:          %d\n",       esds->version);
    printf(" Flags:            0x%06lx\n",  esds->flags);
    printf(" ES ID:            0x%04x\n",   esds->esid);
    printf(" Priority:         0x%02x\n",   esds->stream_priority);
    printf(" objectTypeId:     %d\n",       esds->objectTypeId);
    printf(" streamType:       0x%02x\n",   esds->streamType);
    printf(" bufferSizeDB:     %d\n",       esds->bufferSizeDB);
    
    printf(" maxBitrate:       %d\n",       esds->maxBitrate);
    printf(" avgBitrate:       %d\n",       esds->avgBitrate);
    printf(" decoderConfigLen: %d\n",       esds->decoderConfigLen);
    printf(" decoderConfig:");
    for(i = 0; i < esds->decoderConfigLen; i++)
    {
        if(!(i % 16))
            printf("\n ");
        printf("%02x ", esds->decoderConfig[i]);
    }
    printf("\n");
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// TODO: refactor this so as not to need these ffmpeg routines.
// These are not exposed in ffmpeg's API so we dupe them here.
// AVC helper functions for muxers,
//  * Copyright (c) 2006 Baptiste Coudurier <baptiste.coudurier@smartjog.com>
// This is part of FFmpeg
//  * License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
#define VDA_RB16(x)                          \
((((const uint8_t*)(x))[0] <<  8) |        \
((const uint8_t*)(x)) [1])

#define VDA_RB24(x)                          \
((((const uint8_t*)(x))[0] << 16) |        \
(((const uint8_t*)(x))[1] <<  8) |        \
((const uint8_t*)(x))[2])

#define VDA_RB32(x)                          \
((((const uint8_t*)(x))[0] << 24) |        \
(((const uint8_t*)(x))[1] << 16) |        \
(((const uint8_t*)(x))[2] <<  8) |        \
((const uint8_t*)(x))[3])

static const uint8_t *avc_find_startcode_internal(const uint8_t *p, const uint8_t *end)
{
    const uint8_t *a = p + 4 - ((intptr_t)p & 3);
    
    for (end -= 3; p < a && p < end; p++)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }
    
    for (end -= 3; p < end; p += 4)
    {
        uint32_t x = *(const uint32_t*)p;
        if ((x - 0x01010101) & (~x) & 0x80808080) // generic
        {
            if (p[1] == 0)
            {
                if (p[0] == 0 && p[2] == 1)
                    return p;
                if (p[2] == 0 && p[3] == 1)
                    return p+1;
            }
            if (p[3] == 0)
            {
                if (p[2] == 0 && p[4] == 1)
                    return p+2;
                if (p[4] == 0 && p[5] == 1)
                    return p+3;
            }
        }
    }
    
    for (end += 3; p < end; p++)
    {
        if (p[0] == 0 && p[1] == 0 && p[2] == 1)
            return p;
    }
    
    return end + 3;
}

const uint8_t *avc_find_startcode(const uint8_t *p, const uint8_t *end)
{
    const uint8_t *out= avc_find_startcode_internal(p, end);
    if (p<out && out<end && !out[-1])
        out--;
    return out;
}

const int avc_parse_nal_units(AVIOContext *pb, const uint8_t *buf_in, int size)
{
    const uint8_t *p = buf_in;
    const uint8_t *end = p + size;
    const uint8_t *nal_start, *nal_end;
    
    size = 0;
    nal_start = avc_find_startcode(p, end);
    while (nal_start < end)
    {
        while (!*(nal_start++));
        nal_end = avc_find_startcode(nal_start, end);
        avio_wb32(pb, nal_end - nal_start);
        avio_write(pb, nal_start, nal_end - nal_start);
        size += 4 + nal_end - nal_start;
        nal_start = nal_end;
    }
    return size;
}

const int avc_parse_nal_units_buf(const uint8_t *buf_in, uint8_t **buf, int *size)
{
    AVIOContext *pb;
    int ret = avio_open_dyn_buf(&pb);
    if (ret < 0)
        return ret;
    
    avc_parse_nal_units(pb, buf_in, *size);
    
    av_freep(buf);
    *size = avio_close_dyn_buf(pb, buf);
    return 0;
}

/*
 * if extradata size is greater than 7, then have a valid quicktime
 * avcC atom header.
 *
 *      -: avcC atom header :-
 *  -----------------------------------
 *  1 byte  - version
 *  1 byte  - h.264 stream profile
 *  1 byte  - h.264 compatible profiles
 *  1 byte  - h.264 stream level
 *  6 bits  - reserved set to 63
 *  2 bits  - NAL length
 *            ( 0 - 1 byte; 1 - 2 bytes; 3 - 4 bytes)
 *  3 bit   - reserved
 *  5 bits  - number of SPS
 *  for (i=0; i < number of SPS; i++) {
 *      2 bytes - SPS length
 *      SPS length bytes - SPS NAL unit
 *  }
 *  1 byte  - number of PPS
 *  for (i=0; i < number of PPS; i++) {
 *      2 bytes - PPS length
 *      PPS length bytes - PPS NAL unit
 *  }
 
 how to detect the interlacing used on an existing stream:
 - progressive is signalled by setting
 frame_mbs_only_flag: 1 in the SPS.
 - interlaced is signalled by setting
 frame_mbs_only_flag: 0 in the SPS and
 field_pic_flag: 1 on all frames.
 - paff is signalled by setting
 frame_mbs_only_flag: 0 in the SPS and
 field_pic_flag: 1 on all frames that get interlaced and
 field_pic_flag: 0 on all frames that get progressive.
 - mbaff is signalled by setting
 frame_mbs_only_flag: 0 in the SPS and
 mb_adaptive_frame_field_flag: 1 in the SPS and
 field_pic_flag: 0 on the frames,
 (field_pic_flag: 1 would indicate a normal interlaced frame).
 */
const int isom_write_avcc(AVIOContext *pb, const uint8_t *data, int len)
{
    // extradata from bytestream h264, convert to avcC atom data for bitstream
    if (len > 6)
    {
        /* check for h264 start code */
        if (VDA_RB32(data) == 0x00000001 || VDA_RB24(data) == 0x000001)
        {
            uint8_t *buf=NULL, *end, *start;
            uint32_t sps_size=0, pps_size=0;
            uint8_t *sps=0, *pps=0;
            
            int ret = avc_parse_nal_units_buf(data, &buf, &len);
            if (ret < 0)
                return ret;
            start = buf;
            end = buf + len;
            
            /* look for sps and pps */
            while (buf < end)
            {
                unsigned int size;
                uint8_t nal_type;
                size = VDA_RB32(buf);
                nal_type = buf[4] & 0x1f;
                if (nal_type == 7) /* SPS */
                {
                    sps = buf + 4;
                    sps_size = size;
                }
                else if (nal_type == 8) /* PPS */
                {
                    pps = buf + 4;
                    pps_size = size;
                }
                buf += size + 4;
            }
            assert(sps);
            
            avio_w8(pb, 1); /* version */
            avio_w8(pb, sps[1]); /* profile */
            avio_w8(pb, sps[2]); /* profile compat */
            avio_w8(pb, sps[3]); /* level */
            avio_w8(pb, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
            avio_w8(pb, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */
            
            avio_wb16(pb, sps_size);
            avio_write(pb, sps, sps_size);
            if (pps)
            {
                avio_w8(pb, 1); /* number of pps */
                avio_wb16(pb, pps_size);
                avio_write(pb, pps, pps_size);
            }
            av_free(start);
        }
        else
        {
            avio_write(pb, data, len);
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// GStreamer h264 parser
// Copyright (C) 2005 Michal Benes <michal.benes@itonis.tv>
//           (C) 2008 Wim Taymans <wim.taymans@gmail.com>
// gsth264parse.c:
//  * License as published by the Free Software Foundation; either
//  * version 2.1 of the License, or (at your option) any later version.
typedef struct
{
    const uint8_t *data;
    const uint8_t *end;
    int head;
    uint64_t cache;
} nal_bitstream;

static void
nal_bs_init(nal_bitstream *bs, const uint8_t *data, size_t size)
{
    bs->data = data;
    bs->end  = data + size;
    bs->head = 0;
    // fill with something other than 0 to detect
    //  emulation prevention bytes
    bs->cache = 0xffffffff;
}

static uint32_t
nal_bs_read(nal_bitstream *bs, int n)
{
    uint32_t res = 0;
    int shift;
    
    if (n == 0)
        return res;
    
    // fill up the cache if we need to
    while (bs->head < n)
    {
        uint8_t a_byte;
        bool check_three_byte;
        
        check_three_byte = TRUE;
    next_byte:
        if (bs->data >= bs->end)
        {
            // we're at the end, can't produce more than head number of bits
            n = bs->head;
            break;
        }
        // get the byte, this can be an emulation_prevention_three_byte that we need
        // to ignore.
        a_byte = *bs->data++;
        if (check_three_byte && a_byte == 0x03 && ((bs->cache & 0xffff) == 0))
        {
            // next byte goes unconditionally to the cache, even if it's 0x03
            check_three_byte = FALSE;
            goto next_byte;
        }
        // shift bytes in cache, moving the head bits of the cache left
        bs->cache = (bs->cache << 8) | a_byte;
        bs->head += 8;
    }
    
    // bring the required bits down and truncate
    if ((shift = bs->head - n) > 0)
        res = bs->cache >> shift;
    else
        res = bs->cache;
    
    // mask out required bits
    if (n < 32)
        res &= (1 << n) - 1;
    
    bs->head = shift;
    
    return res;
}

static bool
nal_bs_eos(nal_bitstream *bs)
{
    return (bs->data >= bs->end) && (bs->head == 0);
}

// read unsigned Exp-Golomb code
static int
nal_bs_read_ue(nal_bitstream *bs)
{
    int i = 0;
    
    while (nal_bs_read(bs, 1) == 0 && !nal_bs_eos(bs) && i < 32)
        i++;
    
    return ((1 << i) - 1 + nal_bs_read(bs, i));
}

typedef struct
{
    int profile_idc;
    int level_idc;
    int sps_id;
    
    int chroma_format_idc;
    int separate_colour_plane_flag;
    int bit_depth_luma_minus8;
    int bit_depth_chroma_minus8;
    int qpprime_y_zero_transform_bypass_flag;
    int seq_scaling_matrix_present_flag;
    
    int log2_max_frame_num_minus4;
    int pic_order_cnt_type;
    int log2_max_pic_order_cnt_lsb_minus4;
    
    int max_num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;
    
    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;
    
    int direct_8x8_inference_flag;
    
    int frame_cropping_flag;
    int frame_crop_left_offset;
    int frame_crop_right_offset;
    int frame_crop_top_offset;
    int frame_crop_bottom_offset;
} sps_info_struct;

static void
parseh264_sps(uint8_t *sps, uint32_t sps_size,  int *level, int *profile, bool *interlaced, int32_t *max_ref_frames)
{
    nal_bitstream bs;
    sps_info_struct sps_info = {0};
    
    nal_bs_init(&bs, sps, sps_size);
    
    sps_info.profile_idc  = nal_bs_read(&bs, 8);
    nal_bs_read(&bs, 1);  // constraint_set0_flag
    nal_bs_read(&bs, 1);  // constraint_set1_flag
    nal_bs_read(&bs, 1);  // constraint_set2_flag
    nal_bs_read(&bs, 1);  // constraint_set3_flag
    nal_bs_read(&bs, 4);  // reserved
    sps_info.level_idc    = nal_bs_read(&bs, 8);
    sps_info.sps_id       = nal_bs_read_ue(&bs);
    
    if (sps_info.profile_idc == 100 ||
        sps_info.profile_idc == 110 ||
        sps_info.profile_idc == 122 ||
        sps_info.profile_idc == 244 ||
        sps_info.profile_idc == 44  ||
        sps_info.profile_idc == 83  ||
        sps_info.profile_idc == 86)
    {
        sps_info.chroma_format_idc                    = nal_bs_read_ue(&bs);
        if (sps_info.chroma_format_idc == 3)
            sps_info.separate_colour_plane_flag         = nal_bs_read(&bs, 1);
        sps_info.bit_depth_luma_minus8                = nal_bs_read_ue(&bs);
        sps_info.bit_depth_chroma_minus8              = nal_bs_read_ue(&bs);
        sps_info.qpprime_y_zero_transform_bypass_flag = nal_bs_read(&bs, 1);
        
        sps_info.seq_scaling_matrix_present_flag = nal_bs_read (&bs, 1);
        if (sps_info.seq_scaling_matrix_present_flag)
        {
            /* TODO: unfinished */
        }
    }
    sps_info.log2_max_frame_num_minus4 = nal_bs_read_ue(&bs);
    if (sps_info.log2_max_frame_num_minus4 > 12)
    { // must be between 0 and 12
        // don't early return here - the bits we are using (profile/level/interlaced/ref frames)
        // might still be valid - let the parser go on and pray.
        //return;
    }
    
    sps_info.pic_order_cnt_type = nal_bs_read_ue(&bs);
    if (sps_info.pic_order_cnt_type == 0)
    {
        sps_info.log2_max_pic_order_cnt_lsb_minus4 = nal_bs_read_ue(&bs);
    }
    else if (sps_info.pic_order_cnt_type == 1)
    { // TODO: unfinished
        /*
         delta_pic_order_always_zero_flag = gst_nal_bs_read (bs, 1);
         offset_for_non_ref_pic = gst_nal_bs_read_se (bs);
         offset_for_top_to_bottom_field = gst_nal_bs_read_se (bs);
         
         num_ref_frames_in_pic_order_cnt_cycle = gst_nal_bs_read_ue (bs);
         for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
         offset_for_ref_frame[i] = gst_nal_bs_read_se (bs);
         */
    }
    
    sps_info.max_num_ref_frames             = nal_bs_read_ue(&bs);
    sps_info.gaps_in_frame_num_value_allowed_flag = nal_bs_read(&bs, 1);
    sps_info.pic_width_in_mbs_minus1        = nal_bs_read_ue(&bs);
    sps_info.pic_height_in_map_units_minus1 = nal_bs_read_ue(&bs);
    
    sps_info.frame_mbs_only_flag            = nal_bs_read(&bs, 1);
    if (!sps_info.frame_mbs_only_flag)
        sps_info.mb_adaptive_frame_field_flag = nal_bs_read(&bs, 1);
    
    sps_info.direct_8x8_inference_flag      = nal_bs_read(&bs, 1);
    
    sps_info.frame_cropping_flag            = nal_bs_read(&bs, 1);
    if (sps_info.frame_cropping_flag)
    {
        sps_info.frame_crop_left_offset       = nal_bs_read_ue(&bs);
        sps_info.frame_crop_right_offset      = nal_bs_read_ue(&bs);
        sps_info.frame_crop_top_offset        = nal_bs_read_ue(&bs);
        sps_info.frame_crop_bottom_offset     = nal_bs_read_ue(&bs);
    }
    
    *level = sps_info.level_idc;
    *profile = sps_info.profile_idc;
    *interlaced = !sps_info.frame_mbs_only_flag;
    *max_ref_frames = sps_info.max_num_ref_frames;
}

bool validate_avcC_spc(uint8_t *extradata, uint32_t extrasize, int32_t *max_ref_frames, int *level, int *profile)
{
    // check the avcC atom's sps for number of reference frames and
    // bail if interlaced, VDA does not handle interlaced h264.
    bool interlaced = true;
    uint8_t *spc = extradata + 6;
    uint32_t sps_size = VDA_RB16(spc);
    if (sps_size)
        parseh264_sps(spc+3, sps_size-1, level, profile, &interlaced, max_ref_frames);
    if (interlaced)
        return false;
    return true;
}

//-----------------------------------------------------------------------------------

VideoToolBoxDecoder::VideoToolBoxDecoder()
{
    m_fmt_desc    = NULL;
    m_vt_session  = NULL;
    m_pFormatName = "vtb";
    
    m_queue_depth = 0;
    m_display_queue = NULL;
    m_max_ref_frames = 4;
    pthread_mutex_init(&m_queue_mutex, NULL);
    
    m_convert_bytestream = false;
    m_convert_3byteTo4byteNALSize = false;
    m_DropPictures = false;
    m_sort_time_offset = 0.0;
}

VideoToolBoxDecoder::~VideoToolBoxDecoder()
{
    dispose();
    pthread_mutex_destroy(&m_queue_mutex);
}

const char* VideoToolBoxDecoder::getName()
{
    return m_pFormatName;
}

bool VideoToolBoxDecoder::open(TrackFormat *videoFormat)
{
    mVideoStream = (AVStream*)videoFormat->avStreamContext;
    if(mVideoStream==NULL)
    {
        LOGE("%s","mVideoStream is null");
        return false;
    }
    
    mCodecContext = mVideoStream->codec;
    if(mCodecContext==NULL)
    {
        LOGE("%s","mCodecContext is null");
        return false;
    }
    
    int width  = mCodecContext->width;
    int height = mCodecContext->height;
    
    unsigned int extrasize = mCodecContext->extradata_size; // extra data for codec to use
    uint8_t *extradata = mCodecContext->extradata; // size of extra data
    
    int level = 0;
    int profile = 0;
    int spsLevel = 0;
    int spsProfile = 0;
    
    if (width <= 0 || height <= 0)
    {
        LOGI("%s - bailing with bogus hints, width(%d), height(%d)",
                  __FUNCTION__, width, height);
        return false;
    }
    
    switch (mCodecContext->codec_id) {
        case AV_CODEC_ID_MPEG4:
            if (extrasize)
            {
                AVIOContext *pb;
                quicktime_esds_t *esds;
                
                if (avio_open_dyn_buf(&pb) < 0)
                    return false;
                
                esds = quicktime_set_esds(extradata, extrasize);
                quicktime_write_esds(pb, esds);
                
                // unhook from ffmpeg's extradata
                extradata = NULL;
                // extract the esds atom decoderConfig from extradata
                extrasize = avio_close_dyn_buf(pb, &extradata);
                free(esds->decoderConfig);
                free(esds);
                
                m_fmt_desc = CreateFormatDescriptionFromCodecData(
                                                                  kVTFormatMPEG4Video, width, height, extradata, extrasize, 'esds');
                
                // done with the converted extradata, we MUST free using av_free
                av_free(extradata);
            }
            else
            {
                m_fmt_desc = CreateFormatDescription(kVTFormatMPEG4Video, width, height);          
            }
            m_pFormatName = "vtb-mpeg4";
            break;
        case AV_CODEC_ID_MPEG2VIDEO:
            m_fmt_desc = CreateFormatDescription(kVTFormatMPEG2Video, width, height);
            m_pFormatName = "vtb-mpeg2";
            break;
        case AV_CODEC_ID_H264:
            if (extrasize < 7 || extradata == NULL)
            {
                //m_fmt_desc = CreateFormatDescription(kVTFormatH264, width, height);
                LOGE("%s - avcC atom too data small or missing", __FUNCTION__);
                return false;
            }
            if (extradata[0] == 1)
            {
                // check for interlaced and get number of ref frames
                if (!validate_avcC_spc(extradata, extrasize, &m_max_ref_frames, &spsLevel, &spsProfile))
                    return false;
                
                // overwrite level and profile from the hints
                // if we got something more valid from the extradata
                if (level == 0 && spsLevel > 0)
                    level = spsLevel;
                
                if (profile == 0 && spsProfile > 0)
                    profile = spsProfile;
                
                // we need to check this early, CreateFormatDescriptionFromCodecData will silently fail
                // with a bogus m_fmt_desc returned that crashes on CFRelease.
                if (profile == FF_PROFILE_H264_MAIN && level == 32 && m_max_ref_frames > 4)
                {
                    // Main@L3.2, VTB cannot handle greater than 4 ref frames (ie. flash video)
                    LOGI("%s - Main@L3.2 detected, VTB cannot decode with %d ref frames",
                              __FUNCTION__, m_max_ref_frames);
                    return false;
                }
                
                if (extradata[4] == 0xFE)
                {
                    // video content is from some silly encoder that think 3 byte NAL sizes
                    // are valid, setup to convert 3 byte NAL sizes to 4 byte.
                    extradata[4] = 0xFF;
                    m_convert_3byteTo4byteNALSize = true;
                }
                // valid avcC atom data always starts with the value 1 (version)
                m_fmt_desc = CreateFormatDescriptionFromCodecData(
                                                                  kVTFormatH264, width, height, extradata, extrasize, 'avcC');
                
                LOGI("%s - using avcC atom of size(%d), ref_frames(%d)", __FUNCTION__, extrasize, m_max_ref_frames);
            }
            else
            {
                if ( (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 0 && extradata[3] == 1) ||
                    (extradata[0] == 0 && extradata[1] == 0 && extradata[2] == 1))
                {
                    // video content is from x264 or from bytestream h264 (AnnexB format)
                    // NAL reformating to bitstream format required
                    
                    AVIOContext *pb;
                    if (avio_open_dyn_buf(&pb) < 0)
                        return false;
                    
                    m_convert_bytestream = true;
                    // create a valid avcC atom data from ffmpeg's extradata
                    isom_write_avcc(pb, extradata, extrasize);
                    // unhook from ffmpeg's extradata
                    extradata = NULL;
                    // extract the avcC atom data into extradata getting size into extrasize
                    extrasize = avio_close_dyn_buf(pb, &extradata);
                    
                    // check for interlaced and get number of ref frames
                    if (!validate_avcC_spc(extradata, extrasize, &m_max_ref_frames, &spsLevel, &spsProfile))
                    {
                        av_free(extradata);
                        return false;
                    }
                    
                    // overwrite level and profile from the hints
                    // if we got something more valid from the extradata
                    if (level == 0 && spsLevel > 0)
                        level = spsLevel;
                    
                    if (profile == 0 && spsProfile > 0)
                        profile = spsProfile;
                    
                    // we need to check this early, CreateFormatDescriptionFromCodecData will silently fail
                    // with a bogus m_fmt_desc returned that crashes on CFRelease.
                    if (profile == FF_PROFILE_H264_MAIN && level == 32 && m_max_ref_frames > 4)
                    {
                        // Main@L3.2, VTB cannot handle greater than 4 ref frames (ie. flash video)
                        LOGI("%s - Main@L3.2 detected, VTB cannot decode with %d ref frames",
                                  __FUNCTION__, m_max_ref_frames);
                        av_free(extradata);
                        return false;
                    }
                    
                    // CFDataCreate makes a copy of extradata contents
                    m_fmt_desc = CreateFormatDescriptionFromCodecData(
                                                                      kVTFormatH264, width, height, extradata, extrasize, 'avcC');
                    
                    // done with the new converted extradata, we MUST free using av_free
                    av_free(extradata);
                    LOGI("%s - created avcC atom of size(%d)", __FUNCTION__, extrasize);
                }
                else
                {
                    LOGI("%s - invalid avcC atom data", __FUNCTION__);
                    return false;
                }
            }
            m_pFormatName = "vtb-h264";
            break;
        default:
            return false;
        break;
    }
    
    if(m_fmt_desc == NULL)
    {
        LOGI("%s - created avcC atom of failed", __FUNCTION__);
        m_pFormatName = "";
        return false;
    }
    
    if (m_max_ref_frames == 0)
        m_max_ref_frames = 2;
    
    CreateVTSession(width, height, m_fmt_desc);
    if (m_vt_session == NULL)
    {
        if (m_fmt_desc)
        {
            CFRelease(m_fmt_desc);
            m_fmt_desc = NULL;
        }
        m_pFormatName = "";
        return false;
    }
    
    mFrame = av_frame_alloc();
    mFrame->width = width;
    mFrame->height = height;
    
    m_DropPictures = false;
    m_max_ref_frames = std::min(m_max_ref_frames, 5);
    m_sort_time_offset = (iOSUtils::CurrentHostCounter() * 1000.0) / iOSUtils::CurrentHostFrequency();

    return true;
}

void VideoToolBoxDecoder::dispose()
{
    DestroyVTSession();
    if (m_fmt_desc)
    {
        CFRelease(m_fmt_desc);
        m_fmt_desc = NULL;
    }
    
    av_frame_free(&mFrame);
    
    while (m_queue_depth)
        DisplayQueuePop();
}

void VideoToolBoxDecoder::setDropState(bool bDrop)
{
    m_DropPictures = bDrop;
}

int VideoToolBoxDecoder::decode(AVPacket* videoPacket)
{
    uint8_t* pData = videoPacket->data;
    int iSize = videoPacket->size;
    double dts = videoPacket->dts;
    double pts = videoPacket->pts;

    if (pData)
    {
        OSStatus status;
        double sort_time;
        uint32_t decoderFlags = 0;
        CFDictionaryRef frameInfo = NULL;;
        CMSampleBufferRef sampleBuff = NULL;
        AVIOContext *pb = NULL;
        int demux_size = 0;
        uint8_t *demux_buff = NULL;
        
        if (m_convert_bytestream)
        {
            // convert demuxer packet from bytestream (AnnexB) to bitstream
            if(avio_open_dyn_buf(&pb) < 0)
                return -1;
            
            avc_parse_nal_units(pb, pData, iSize);
            demux_size = avio_close_dyn_buf(pb, &demux_buff);
            sampleBuff = CreateSampleBufferFrom(m_fmt_desc, demux_buff, demux_size);
        }
        else if (m_convert_3byteTo4byteNALSize)
        {
            // convert demuxer packet from 3 byte NAL sizes to 4 byte
            if (avio_open_dyn_buf(&pb) < 0)
                return -1;
            
            uint32_t nal_size;
            uint8_t *end = pData + iSize;
            uint8_t *nal_start = pData;
            while (nal_start < end)
            {
                nal_size = VDA_RB24(nal_start);
                avio_wb32(pb, nal_size);
                nal_start += 3;
                avio_write(pb, nal_start, nal_size);
                nal_start += nal_size;
            }
            
            demux_size = avio_close_dyn_buf(pb, &demux_buff);
            sampleBuff = CreateSampleBufferFrom(m_fmt_desc, demux_buff, demux_size);
        }
        else
        {
            sampleBuff = CreateSampleBufferFrom(m_fmt_desc, pData, iSize);
        }
        
        if (!sampleBuff)
        {
            if (demux_size)
                av_free(demux_buff);
            LOGI("%s - CreateSampleBufferFrom failed", __FUNCTION__);
            return -1;
        }
        
        sort_time = (iOSUtils::CurrentHostCounter() * 1000.0) / iOSUtils::CurrentHostFrequency();
        frameInfo = CreateDictionaryWithDisplayTime(sort_time - m_sort_time_offset, dts, pts);
        
        if (m_DropPictures)
        {
            decoderFlags = kVTDecoderDecodeFlags_DontEmitFrame;
        }
        
        // submit for decoding
        status = VTDecompressionSessionDecodeFrame(m_vt_session, sampleBuff, decoderFlags, frameInfo, 0);
        if (status != kVTDecoderNoErr)
        {
            LOGI("%s - VTDecompressionSessionDecodeFrame returned(%d)",
                 __FUNCTION__, (int)status);
            CFRelease(frameInfo);
            CFRelease(sampleBuff);
            if (demux_size)
                av_free(demux_buff);
            return -1;
            // VTDecompressionSessionDecodeFrame returned 8969 (codecBadDataErr)
            // VTDecompressionSessionDecodeFrame returned -12350
            // VTDecompressionSessionDecodeFrame returned -12902
            // VTDecompressionSessionDecodeFrame returned -12911
        }
        
        // wait for decoding to finish
        status = VTDecompressionSessionWaitForAsynchronousFrames(m_vt_session);
        if (status != kVTDecoderNoErr)
        {
            LOGI("%s - VTDecompressionSessionWaitForAsynchronousFrames returned(%d)",
                 __FUNCTION__, (int)status);
            CFRelease(frameInfo);
            CFRelease(sampleBuff);
            if (demux_size)
                av_free(demux_buff);
            return -1;
        }
        
        CFRelease(frameInfo);
        CFRelease(sampleBuff);
        if (demux_size)
            av_free(demux_buff);
        
        return videoPacket->size;
    }
    
    return 0;
}

void VideoToolBoxDecoder::flush()
{
    // flush decoder
    VTDecompressionSessionWaitForAsynchronousFrames(m_vt_session);
    
    while (m_queue_depth)
        DisplayQueuePop();
    
    m_sort_time_offset = (iOSUtils::CurrentHostCounter() * 1000.0) / iOSUtils::CurrentHostFrequency();
}

AVFrame* VideoToolBoxDecoder::getFrame()
{
    if (m_queue_depth < m_max_ref_frames)
        return NULL;
    
    pthread_mutex_lock(&m_queue_mutex);
    mFrame->pts = m_display_queue->pts;
    mFrame->width = m_display_queue->width;
    mFrame->height = m_display_queue->height;
    mFrame->format = AV_PIX_FMT_YUV420P;
    mFrame->opaque = m_display_queue->pixel_buffer_ref;
    m_display_queue->pixel_buffer_ref = NULL;
    pthread_mutex_unlock(&m_queue_mutex);

    // now we can pop the top frame
    DisplayQueuePop();
    
    static double old_pts;
    if (mFrame->pts < old_pts)
        LOGD("%s - VTBDecoderDecode pts(%f), old_pts(%f)", __FUNCTION__, (double)mFrame->pts, old_pts);
    old_pts = mFrame->pts;
    
    return mFrame;
}

void VideoToolBoxDecoder::DisplayQueuePop(void)
{
    if (!m_display_queue || m_queue_depth == 0)
        return;
    
    // pop the top frame off the queue
    pthread_mutex_lock(&m_queue_mutex);
    frame_queue *top_frame = m_display_queue;
    m_display_queue = m_display_queue->nextframe;
    m_queue_depth--;
    pthread_mutex_unlock(&m_queue_mutex);
    
    // and release it
    if (top_frame->pixel_buffer_ref)
        CVBufferRelease(top_frame->pixel_buffer_ref);
    free(top_frame);
}

void VideoToolBoxDecoder::CreateVTSession(int width, int height, CMFormatDescriptionRef fmt_desc)
{
    VTDecompressionSessionRef vt_session = NULL;
    CFMutableDictionaryRef destinationPixelBufferAttributes;
    VTDecompressionOutputCallback outputCallback;
    OSStatus status;
    
    double scale = 0.0;
    
    // decoding, scaling and rendering above 1920 x 800 runs into
    // some bandwidth limit. detect and scale down to reduce
    // the bandwidth requirements.
    int width_clamp = 1280;
    if ((width * height) > (1920 * 800))
        width_clamp = 960;
    
    // for retina devices it should be safe [tm] to
    // loosen the clamp a bit to 1280 pixels width
    if (iOSUtils::DeviceHasRetina(scale))
        width_clamp = 1280;
    
    int new_width = CheckNP2(width);
    if (width != new_width)
    {
        // force picture width to power of two and scale up height
        // we do this because no GL_UNPACK_ROW_LENGTH in OpenGLES
        // and the CVPixelBufferPixel gets created using some
        // strange alignment when width is non-standard.
        double w_scaler = (double)new_width / width;
        width = new_width;
        height = height * w_scaler;
    }
    // scale output pictures down to 720p size for display
    if (width > width_clamp)
    {
        double w_scaler = (float)width_clamp / width;
        width = width_clamp;
        height = height * w_scaler;
    }

    destinationPixelBufferAttributes = CFDictionaryCreateMutable(
                                                                 NULL, // CFAllocatorRef allocator
                                                                 0,    // CFIndex capacity
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);
    
    // The recommended pixel format choices are
    //  kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange or kCVPixelFormatType_32BGRA.
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferPixelFormatTypeKey, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferWidthKey, width);
    CFDictionarySetSInt32(destinationPixelBufferAttributes,
                          kCVPixelBufferHeightKey, height);
    //CFDictionarySetValue(destinationPixelBufferAttributes,
    //  kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);
    
    outputCallback.callback = VTDecoderCallback;
    outputCallback.refcon = this;
    
    status = VTDecompressionSessionCreate(
                                          NULL, // CFAllocatorRef allocator
                                          fmt_desc,
                                          NULL, // CFTypeRef sessionOptions
                                          destinationPixelBufferAttributes,
                                          &outputCallback,
                                          &vt_session);
    if (status != noErr)
    {
        m_vt_session = NULL;
        LOGE("%s - failed with status = (%d)", __FUNCTION__, (int)status);
    }
    else
    {
        //vtdec_session_dump_properties(vt_session);
        m_vt_session = (void*)vt_session;
    }
    
    CFRelease(destinationPixelBufferAttributes);
}

void VideoToolBoxDecoder::DestroyVTSession(void)
{
    if (m_vt_session)
    {
        VTDecompressionSessionInvalidate((VTDecompressionSessionRef)m_vt_session);
        CFRelease((VTDecompressionSessionRef)m_vt_session);
        m_vt_session = NULL;
    }
}



void
VideoToolBoxDecoder::VTDecoderCallback(
                                              void               *refcon,
                                              CFDictionaryRef    frameInfo,
                                              OSStatus           status,
                                              UInt32             infoFlags,
                                              CVBufferRef        imageBuffer)
{
    // This is an sync callback due to VTDecompressionSessionWaitForAsynchronousFrames
    VideoToolBoxDecoder *ctx = (VideoToolBoxDecoder*)refcon;
    
    if (status != kVTDecoderNoErr)
    {
        //CLog::Log(LOGDEBUG, "%s - status error (%d)", __FUNCTION__, (int)status);
        return;
    }
    if (imageBuffer == NULL)
    {
        //CLog::Log(LOGDEBUG, "%s - imageBuffer is NULL", __FUNCTION__);
        return;
    }
    OSType format_type = CVPixelBufferGetPixelFormatType(imageBuffer);
    if (format_type != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)
    {
        LOGE("%s - imageBuffer format is not 'YUV420P',is reporting 0x%x",
                  "VTDecoderCallback", (int)format_type);
        return;
    }
    if (kVTDecodeInfo_FrameDropped & infoFlags)
    {
            LOGD("%s - frame dropped", __FUNCTION__);
        return;
    }
    
    // allocate a new frame and populate it with some information.
    // this pointer to a frame_queue type keeps track of the newest decompressed frame
    // and is then inserted into a linked list of frame pointers depending on the display time
    // parsed out of the bitstream and stored in the frameInfo dictionary by the client
    frame_queue *newFrame = (frame_queue*)calloc(sizeof(frame_queue), 1);
    newFrame->nextframe = NULL;
    if (CVPixelBufferIsPlanar(imageBuffer) )
    {
        newFrame->width  = CVPixelBufferGetWidthOfPlane(imageBuffer, 0);
        newFrame->height = CVPixelBufferGetHeightOfPlane(imageBuffer, 0);
    }
    else
    {
        newFrame->width  = CVPixelBufferGetWidth(imageBuffer);
        newFrame->height = CVPixelBufferGetHeight(imageBuffer);
    }
    newFrame->pixel_buffer_format = format_type;
    newFrame->pixel_buffer_ref = CVBufferRetain(imageBuffer);
    GetFrameDisplayTimeFromDictionary(frameInfo, newFrame);
    
    // if both dts or pts are good we use those, else use decoder insert time for frame sort
    if ((newFrame->pts != AV_NOPTS_VALUE) || (newFrame->dts != AV_NOPTS_VALUE))
    {
        // if pts is borked (stupid avi's), use dts for frame sort
        if (newFrame->pts == AV_NOPTS_VALUE)
            newFrame->sort_time = newFrame->dts;
        else
            newFrame->sort_time = newFrame->pts;
    }
    
    // since the frames we get may be in decode order rather than presentation order
    // our hypothetical callback places them in a queue of frames which will
    // hold them in display order for display on another thread
    pthread_mutex_lock(&ctx->m_queue_mutex);
    //
    frame_queue *queueWalker = ctx->m_display_queue;
    if (!queueWalker || (newFrame->sort_time < queueWalker->sort_time))
    {
        // we have an empty queue, or this frame earlier than the current queue head.
        newFrame->nextframe = queueWalker;
        ctx->m_display_queue = newFrame;
    }
    else
    {
        // walk the queue and insert this frame where it belongs in display order.
        bool frameInserted = false;
        frame_queue *nextFrame = NULL;
        //
        while (!frameInserted)
        {
            nextFrame = queueWalker->nextframe;
            if (!nextFrame || (newFrame->sort_time < nextFrame->sort_time))
            {
                // if the next frame is the tail of the queue, or our new frame is earlier.
                newFrame->nextframe = nextFrame;
                queueWalker->nextframe = newFrame;
                frameInserted = true;
            }
            queueWalker = nextFrame;
        }
    }
    ctx->m_queue_depth++;
    //
    pthread_mutex_unlock(&ctx->m_queue_mutex);	
}

void* call_C_VTBDecoder_Constructor()
{
    return new VideoToolBoxDecoder;
}

void call_C_VTBDecoder_Destructor(void *vtb)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    
    delete vtbd;
}

bool call_C_VTBDecoder_open(void *vtb, TrackFormat *videoFormat)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->open(videoFormat);
}

void call_C_VTBDecoder_dispose(void *vtb)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    vtbd->dispose();
}

int call_C_VTBDecoder_decode(void *vtb, AVPacket* videoPacket)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->decode(videoPacket);
}

AVFrame* call_C_VTBDecoder_getFrame(void *vtb)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->getFrame();
}

const char* call_C_VTBDecoder_getName(void *vtb)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->getName();
}

void call_C_VTBDecoder_flush(void *vtb)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->flush();
}

void call_C_VTBDecoder_setDropState(void *vtb, bool bDrop)
{
    VideoToolBoxDecoder *vtbd = (VideoToolBoxDecoder*)vtb;
    return vtbd->setDropState(bDrop);
}
