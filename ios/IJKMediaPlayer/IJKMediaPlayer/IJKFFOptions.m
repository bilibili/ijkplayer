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

    options.frameBufferCount        = 3;
    options.maxFps                  = 30;
    options.frameDrop               = 0;
    options.pauseInBackground       = YES;

    options.timeout                 = 30 * 1000 * 1000; // 30 seconds
    options.userAgent               = @"";
    options.videotoolboxEnabled     = YES;
    options.frameMaxWidth           = 960;
    options.autoReconnect           = YES;
    options.reportPlayInfo          = YES;
    options.sourceType  = IJKMPMovieSourceTypeLowDelayLiveStreaming;
    options.cache = 6000;//default 6s
    options.isEnableAudio = YES;

    return options;
}

- (void)applyTo:(IjkMediaPlayer *)mediaPlayer
{
    [self logOptions];

    [self setCodecOption:@"skip_loop_filter"
               withInt64:self.skipLoopFilter
                      to:mediaPlayer];
    [self setCodecOption:@"skip_frame"
               withInt64:self.skipFrame
                      to:mediaPlayer];

    ijkmp_set_picture_queue_capicity(mediaPlayer, _frameBufferCount);
    ijkmp_set_max_fps(mediaPlayer, _maxFps);
    ijkmp_set_framedrop(mediaPlayer, _frameDrop);
    ijkmp_ios_set_videotoolbox_enabled(mediaPlayer, _videotoolboxEnabled);
    ijkmp_ios_set_frame_max_width(mediaPlayer, _frameMaxWidth);

    if (self.autoReconnect == NO) {
        [self setFormatOption:@"reconnect" withInt64:0 to:mediaPlayer];
    } else {
        [self setFormatOption:@"reconnect" withInt64:1 to:mediaPlayer];
    }

#if 0
    if (self.timeout > 0) {
        [self setFormatOption:@"timeout"
                    withInt64:self.timeout
                           to:mediaPlayer];
    }
#endif

    if ([self.userAgent isEqualToString:@""] == NO) {
        [self setFormatOption:@"user-agent" withString:self.userAgent to:mediaPlayer];
    }
}

- (void)logOptions
{
    NSMutableString *echo = [[NSMutableString alloc] init];
    [echo appendString:@"========================================\n"];
    [echo appendString:@"= FFmpeg options:\n"];
    [echo appendFormat:@"= skip_loop_filter: %@\n",   [IJKFFOptions getDiscardString:self.skipLoopFilter]];
    [echo appendFormat:@"= skipFrame:        %@\n",   [IJKFFOptions getDiscardString:self.skipFrame]];
    [echo appendFormat:@"= frameBufferCount: %d\n",   self.frameBufferCount];
    [echo appendFormat:@"= maxFps:           %d\n",   self.maxFps];
    [echo appendFormat:@"= timeout:          %lld\n", self.timeout];
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

- (void)setFormatOption:(NSString *)optionName
              withInt64:(int64_t)value
                     to:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_format_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%lld", value] UTF8String]);
}

- (void)setFormatOption:(NSString *)optionName
              withString:(NSString*)value
                     to:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_format_option(mediaPlayer,
                            [optionName UTF8String],
                            [value UTF8String]);
}


- (void)setCodecOption:(NSString *)optionName
             withInt64:(int64_t)value
                    to:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_codec_option(mediaPlayer,
                           [optionName UTF8String],
                           [[NSString stringWithFormat:@"%lld", value] UTF8String]);
}

@end
