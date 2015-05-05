//
//  MediaDemuxer.h
//  MediaPlayer
//
//  Created by 施灵凯 on 15/3/20.
//  Copyright (c) 2015年 bolome. All rights reserved.
//

#ifndef __MediaPlayer__MediaDemuxer__
#define __MediaPlayer__MediaDemuxer__

#include <stdint.h>

extern "C" {
#include "libavformat/avformat.h"
}

struct TrackFormat
{
    //common
    int trackIndex;
    char mime[64];
    
    //video
    int width;
    int height;
    int frameRate;
    
    //audio
    int sampleRate;
    int channelCount;
    int sampleFormat;
    int64_t channelLayout;
    
    //common
    int64_t durationUs;
    int64_t avStreamContext;
    
    TrackFormat()
    {
        trackIndex = 0;
        mime[0] = '\0';
        
        width = 0;
        height = 0;
        frameRate = 0;
        
        sampleRate = 0;
        channelCount = 0;
        sampleFormat = 0;
        channelLayout = 0;
        
        durationUs = 0;
    }
};

#endif /* defined(__MediaPlayer__MediaDemuxer__) */
