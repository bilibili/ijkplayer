//
//  IJKFFOptions.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-10-17.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKFFOptions.h"
#include "ijkplayer/ios/ijkplayer_ios.h"

@implementation IJKFFOptions {
    NSMutableDictionary *_optionCategories;

    NSMutableDictionary *_playerOptions;
    NSMutableDictionary *_formatOptions;
    NSMutableDictionary *_codecOptions;
    NSMutableDictionary *_swsOptions;
}

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
    options.videoToolboxEnabled     = NO;
    options.frameMaxWidth           = 960;
    options.autoReconnect           = YES;

    return options;
}

- (id)init
{
    self = [super init];
    if (self) {
        _playerOptions      = [[NSMutableDictionary alloc] init];
        _formatOptions      = [[NSMutableDictionary alloc] init];
        _codecOptions       = [[NSMutableDictionary alloc] init];
        _swsOptions         = [[NSMutableDictionary alloc] init];

        _optionCategories   = [[NSMutableDictionary alloc] init];
        _optionCategories[@(IJKMP_OPT_CATEGORY_PLAYER)] = _playerOptions;
        _optionCategories[@(IJKMP_OPT_CATEGORY_FORMAT)] = _formatOptions;
        _optionCategories[@(IJKMP_OPT_CATEGORY_CODEC)]  = _codecOptions;
        _optionCategories[@(IJKMP_OPT_CATEGORY_SWS)]    = _swsOptions;
    }
    return self;
}

- (void)applyTo:(IjkMediaPlayer *)mediaPlayer
{
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_CODEC, "skip_loop_filter", _skipLoopFilter);
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_CODEC, "skip_frame",       _skipFrame);

    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "max-fps",                         _maxFps);
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "framedrop",                       _frameDrop);
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "video-pictq-size",                _frameBufferCount);
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "videotoolbox",                    _videoToolboxEnabled);
    ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_PLAYER, "videotoolbox-max-frame-width",    _frameMaxWidth);

    if (self.autoReconnect == NO) {
        ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_FORMAT, "reconnect", 0);
    } else {
        ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_FORMAT, "reconnect", 1);
    }

    if (self.timeout > 0) {
        ijkmp_set_option_int(mediaPlayer, IJKMP_OPT_CATEGORY_FORMAT, "timeout", self.timeout);
    }

    if ([self.userAgent isEqualToString:@""] == NO) {
        ijkmp_set_option(mediaPlayer, IJKMP_OPT_CATEGORY_FORMAT, "user-agent", [self.userAgent UTF8String]);
    }

    [_optionCategories enumerateKeysAndObjectsUsingBlock:^(id categoryKey, id categoryDict, BOOL *stopOuter) {
        [categoryDict enumerateKeysAndObjectsUsingBlock:^(id optKey, id optValue, BOOL *stop) {
            if ([optValue isKindOfClass:[NSNumber class]]) {
                ijkmp_set_option_int(mediaPlayer,
                                     [categoryKey integerValue],
                                     [optKey UTF8String],
                                     [optValue integerValue]);
            } else if ([optValue isKindOfClass:[NSString class]]) {
                ijkmp_set_option(mediaPlayer,
                                 [categoryKey integerValue],
                                 [optKey UTF8String],
                                 [optValue UTF8String]);
            }
        }];
    }];
}

- (void)setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category
{
    if (!key)
        return;

    NSMutableDictionary *options = [_optionCategories objectForKey:@(category)];
    if (options) {
        if (value) {
            [options setObject:value forKey:key];
        } else {
            [options removeObjectForKey:key];
        }
    }
}

- (void)setOptionIntValue:(NSInteger)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category
{
    if (!key)
        return;

    NSMutableDictionary *options = [_optionCategories objectForKey:@(category)];
    if (options) {
        if (value) {
            [options setObject:@(value) forKey:key];
        } else {
            [options removeObjectForKey:key];
        }
    }
}

@end
