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
    IJKMPET_CURRENT_POSITION_UPDATE = 510,
    IJKMPET_PLAYBACK_STATE_CHANGED  = 700,
};

typedef NS_ENUM(NSInteger, IJKMPErrorCode){
    IJKMPEC_SNAPSHOT                = -480,
};


typedef void(^OnSnapshotBlock) (UIImage* __nullable image , NSError* __nullable error );

@class IJKFFMediaPlayer;

@protocol IJKMPEventHandler <NSObject>

@required
- (void) onEvent4Player:(IJKFFMediaPlayer *)player withType:(int)waht andArg1:(int)arg1 andArg2:(int)arg2 andExtra:(void *)extra;

@end

@interface IJKFFMediaPlayer : NSObject<IJKCVPBViewProtocol, IJKSDLGLViewProtocol>


@property (nonatomic) BOOL ignoreAudioInterrupt;
@property (nonatomic) BOOL cacheSnapshot;

- (instancetype)init;
- (instancetype)initWithFbo;
- (int) setDataSource:(NSString *)url;
- (int) prepareAsync;
- (int) start;
- (int) stop;
- (int) pause;
- (int) reset;
- (BOOL) isPlaying;
- (void) shutdown;

- (void) takeSnapshot:(OnSnapshotBlock) block;
- (long) getCurrentPosition;
- (long) getDuration;
- (int)  seekTo:(long) msec;

- (void) setLoop:(int) loop;
- (int)  getLoop;

- (void) setSpeed:(float) speed;

- (void) setPlaybackVolume:(float)volume;
- (float) playbackVolume;

- (void) setupCVPixelBufferView:(id<IJKCVPBViewProtocol>) view;

- (void) setStreamSelected:(int) stream selected:(BOOL) selected;

- (float) getFloatProperty:(int) property defalut:(float) value;
- (int64_t) getLongProperty:(int) property default:(int64_t) value;

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
