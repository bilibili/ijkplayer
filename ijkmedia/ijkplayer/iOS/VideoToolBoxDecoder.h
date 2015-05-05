//
//  VideoToolBoxDecoder.h
//  MediaPlayer
//
//  Created by 施灵凯 on 15/4/15.
//  Copyright (c) 2015年 bolome. All rights reserved.
//

#ifndef __MediaPlayer__VideoToolBoxDecoder__
#define __MediaPlayer__VideoToolBoxDecoder__

#include <stdio.h>

#include <CoreVideo/CoreVideo.h>
#include <CoreMedia/CoreMedia.h>

#include "VideoDecoder.h"

// tracks a frame in and output queue in display order
typedef struct frame_queue {
    double              dts;
    double              pts;
    int                 width;
    int                 height;
    double              sort_time;
    FourCharCode        pixel_buffer_format;
    CVPixelBufferRef    pixel_buffer_ref;
    struct frame_queue  *nextframe;
} frame_queue;

class VideoToolBoxDecoder : public VideoDecoder
{
public:
    VideoToolBoxDecoder();
    ~VideoToolBoxDecoder();
    
    bool open(TrackFormat *videoFormat);
    
    void dispose();
    
    int decode(AVPacket* videoPacket);
    
    AVFrame* getFrame();
    
    const char* getName();
    
    void flush();
    
    void setDropState(bool bDrop);
    
private:
    void DisplayQueuePop(void);
    void CreateVTSession(int width, int height, CMFormatDescriptionRef fmt_desc);
    void DestroyVTSession(void);
    static void VTDecoderCallback(void *refcon, CFDictionaryRef frameInfo, OSStatus status, UInt32 infoFlags, CVBufferRef imageBuffer);
    
    AVStream* mVideoStream;
    AVCodecContext *mCodecContext;
    
    void              *m_vt_session;    // opaque videotoolbox session
    CMFormatDescriptionRef m_fmt_desc;
    
    const char        *m_pFormatName;
    bool              m_DropPictures;
    AVFrame           *mFrame;
    
    double            m_sort_time_offset;
    int32_t           m_max_ref_frames;
    pthread_mutex_t   m_queue_mutex;    // mutex protecting queue manipulation
    frame_queue       *m_display_queue; // display-order queue - next display frame is always at the queue head
    int32_t           m_queue_depth;    // we will try to keep the queue depth at m_max_ref_frames
    
    bool              m_convert_bytestream;
    bool              m_convert_3byteTo4byteNALSize;
    
    
};

extern "C"
{
    void* call_C_VTBDecoder_Constructor();
    void call_C_VTBDecoder_Destructor(void *vtb);
    
    bool call_C_VTBDecoder_open(void *vtb, TrackFormat *videoFormat);
    void call_C_VTBDecoder_dispose(void *vtb);
    
    int call_C_VTBDecoder_decode(void *vtb, AVPacket* videoPacket);
    AVFrame* call_C_VTBDecoder_getFrame(void *vtb);
    
    const char* call_C_VTBDecoder_getName(void *vtb);
    
    void call_C_VTBDecoder_flush(void *vtb);
    
    void call_C_VTBDecoder_setDropState(void *vtb, bool bDrop);
}

#endif /* defined(__MediaPlayer__VideoToolBoxDecoder__) */
