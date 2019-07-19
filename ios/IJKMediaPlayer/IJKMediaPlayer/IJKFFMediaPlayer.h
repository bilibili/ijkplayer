/*
 * IJKFFMoviePlayerDef.m
 *
 * Copyright (c) 2019 Befovy <befovy@gmail.com>
 *
 * This file is part of fijkPlayer.
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

#import <Foundation/Foundation.h>
#import "IJKFFOptions.h"
#import "IJKSDLGLViewProtocol.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, IJKMPEventType) {
    IJKMPET_FLUSH                   = 0,
    IJKMPET_ERROR                   = 100,
    IJKMPET_PREPARED                = 200,
    IJKMPET_COMPLETED               = 300,
    IJKMPET_VIDEO_SIZE_CHANGED      = 400,
    IJKMPET_SAR_CHANGED             = 401,
    IJKMPET_VIDEO_RENDERING_START   = 402,
    IJKMPET_AUDIO_RENDERING_START   = 403,
    IJKMPET_VIDEO_ROTATION_CHANGED  = 404,
    IJKMPET_BUFFERING_START         = 500,
    IJKMPET_BUFFERING_END           = 501,
    IJKMPET_BUFFERING_UPDATE        = 502,
    IJKMPET_PLAYBACK_STATE_CHANGED  = 700,
};


@class IJKFFMediaPlayer;

@protocol IJKMPEventHandler <NSObject>

@required
- (void) onEvent4Player:(IJKFFMediaPlayer *)player withType:(int)waht andArg1:(int)arg1 andArg2:(int)arg2 andExtra:(void *)extra;

@end

@interface IJKFFMediaPlayer : NSObject<IJKSDLGLViewProtocol>

@property (readonly, nonatomic) int videoWidth;
@property (readonly, nonatomic) int videoHeight;
@property (readonly, nonatomic) int videoSarNum;
@property (readonly, nonatomic) int videoSarDen;

- (IJKFFMediaPlayer *)init;

- (int) setDataSource:(NSString *)url;
- (int) prepareAsync;
- (int) start;
- (int) stop;
- (int) pause;
- (int) reset;
- (BOOL) isPlaying;
- (void) shutdown;

- (long) getCurrentPosition;
- (long) getDuration;
- (int) seekTo:(long) msec;


- (void) setupCVPixelBufferView:(id<IJKCVPBViewProtocol>) view;

- (void) setOptionValue:(NSString *)value
                forKey:(NSString *)key
            ofCategory:(IJKFFOptionCategory)category;

- (void) setOptionIntValue:(int64_t)value
                   forKey:(NSString *)key
               ofCategory:(IJKFFOptionCategory)category;


- (void) addIJKMPEventHandler:(id<IJKMPEventHandler>) handler;
- (void) removeIJKMPEventHandler:(id<IJKMPEventHandler>) handler;

@end

NS_ASSUME_NONNULL_END
