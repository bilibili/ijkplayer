//
//  VideoDecoder.h
//  MediaPlayer
//
//  Created by 施灵凯 on 15/3/24.
//  Copyright (c) 2015年 bolome. All rights reserved.
//

#ifndef __MediaPlayer__VideoDecoder__
#define __MediaPlayer__VideoDecoder__

#include <stdio.h>

extern "C" {
#include "libavformat/avformat.h"
}

#include "MediaDemuxer.h"

class VideoDecoder
{
public:
    virtual ~VideoDecoder() {}
    
    /*
     * Open the decoder, returns true on success
     */
    virtual bool open(TrackFormat *videoFormat) = 0;
    
    /*
     * Dispose, Free all resources
     */
    virtual void dispose() = 0;
    
    /*
     * returns bytes used or -1 on error
     *
     */
    virtual int decode(AVPacket* videoPacket) = 0;
    
    /*
     * the data is valid until the next Decode call
     */
    virtual AVFrame* getFrame() = 0;
    
    /*
     *
     * should return codecs name
     */
    virtual const char* getName() = 0;
    
    /*
     * flush the decoder.
     */
    virtual void flush() = 0;
    
    /*
     * will be called by video player indicating if a frame will eventually be dropped
     * codec can then skip actually decoding the data, just consume the data set picture headers
     */
    virtual void setDropState(bool bDrop);
};

#endif /* defined(__MediaPlayer__VideoDecoder__) */
