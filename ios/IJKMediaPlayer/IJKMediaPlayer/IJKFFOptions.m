//
//  IJKFFOptions.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-10-17.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKFFOptions.h"
#include "ijkplayer/ios/ijkplayer_ios.h"

@implementation IJKFFOptions

+(IJKFFOptions *)optionsByDefault
{
    IJKFFOptions *options = [[IJKFFOptions alloc] init];

    options.skipLoopFilter  = IJK_AVDISCARD_ALL;
    options.skipFrame       = IJK_AVDISCARD_NONREF;

    options.frameBufferCount = 3;

    return options;
}

-(void)applyTo:(IjkMediaPlayer *)mediaPlayer
{
    [self setCodecOption:@"skip_loop_filter"
             withInteger:self.skipLoopFilter
                      to:mediaPlayer];
    [self setCodecOption:@"skip_frame"
             withInteger:self.skipFrame
                      to:mediaPlayer];

    ijkmp_set_picture_queue_capicity(mediaPlayer, _frameBufferCount);
}

-(void)setCodecOption:(NSString *)optionName
          withInteger:(NSInteger)value
                   to:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_codec_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%d", (int)value] UTF8String]);
}

@end
