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

+ (IJKFFOptions *)optionsByDefault
{
    IJKFFOptions *options = [[IJKFFOptions alloc] init];

    options.skipLoopFilter  = IJK_AVDISCARD_ALL;
    options.skipFrame       = IJK_AVDISCARD_NONREF;

    options.frameBufferCount = 3;
    options.maxFps           = 30;

    return options;
}

- (void)applyTo:(IjkMediaPlayer *)mediaPlayer
{
    [self logOptions];

    [self setCodecOption:@"skip_loop_filter"
             withInteger:self.skipLoopFilter
                      to:mediaPlayer];
    [self setCodecOption:@"skip_frame"
             withInteger:self.skipFrame
                      to:mediaPlayer];

    ijkmp_set_picture_queue_capicity(mediaPlayer, _frameBufferCount);
    ijkmp_set_max_fps(mediaPlayer, _maxFps);
}

- (void)logOptions
{
    NSMutableString *echo = [[NSMutableString alloc] init];
    [echo appendString:@"========================================\n"];
    [echo appendString:@"= FFmpeg options:\n"];
    [echo appendFormat:@"= skip_loop_filter: %@\n", [IJKFFOptions getDiscardString:self.skipLoopFilter]];
    [echo appendFormat:@"= skipFrame:        %@\n", [IJKFFOptions getDiscardString:self.skipFrame]];
    [echo appendFormat:@"= frameBufferCount: %d\n", self.frameBufferCount];
    [echo appendFormat:@"= maxFps:           %d\n", self.maxFps];
    [echo appendString:@"========================================\n"];
    NSLog(@"%@", echo);
}

+ (NSString *)getDiscardString:(IJKAVDiscard)discard
{
    switch (discard) {
        case IJK_AVDISCARD_NONE:
            return @"avdiscard none";
        case IJK_AVDISCARD_DEFAULT:
            return @"avdiscard default";
        case IJK_AVDISCARD_NONREF:
            return @"avdiscard nonref";
        case IJK_AVDISCARD_BIDIR:
            return @"avdicard bidir;";
        case IJK_AVDISCARD_NONKEY:
            return @"avdicard nonkey";
        case IJK_AVDISCARD_ALL:
            return @"avdicard all;";
        default:
            return @"avdiscard unknown";
    }
}

- (void)setCodecOption:(NSString *)optionName
          withInteger:(NSInteger)value
                   to:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_codec_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%d", (int)value] UTF8String]);
}

@end
