/*
 * IJKMediaPlayback.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
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
#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>


@protocol IJKMediaPlayback;


#pragma mark IJKMediaPlayback

@protocol IJKMediaPlayback <NSObject>

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;
- (BOOL)isPlaying;
- (void)shutdown;
- (void)setPauseInBackground:(BOOL)pause;

@property(nonatomic, readonly)  UIView *view;
@property(nonatomic)            NSTimeInterval currentPlaybackTime;
@property(nonatomic, readonly)  NSTimeInterval duration;
@property(nonatomic, readonly)  NSTimeInterval playableDuration;
@property(nonatomic, readonly)  NSInteger bufferingProgress;

@property(nonatomic, readonly)  BOOL isPreparedToPlay;
@property(nonatomic, readonly)  MPMoviePlaybackState playbackState;
@property(nonatomic, readonly)  MPMovieLoadState loadState;

@property(nonatomic, readonly) int64_t numberOfBytesTransferred;

@property(nonatomic) MPMovieControlStyle controlStyle;
@property(nonatomic) MPMovieScalingMode scalingMode;
@property(nonatomic) BOOL shouldAutoplay;

@property (nonatomic) BOOL allowsMediaAirPlay;
@property (nonatomic) BOOL isDanmakuMediaAirPlay;
@property (nonatomic, readonly) BOOL airPlayMediaActive;

- (UIImage *)thumbnailImageAtCurrentTime;

#pragma mark Notifications

#ifdef __cplusplus
#define IJK_EXTERN extern "C" __attribute__((visibility ("default")))
#else
#define IJK_EXTERN extern __attribute__((visibility ("default")))
#endif

IJK_EXTERN NSString *const IJKMediaPlaybackIsPreparedToPlayDidChangeNotification;

IJK_EXTERN NSString *const IJKMoviePlayerLoadStateDidChangeNotification;
IJK_EXTERN NSString *const IJKMoviePlayerPlaybackDidFinishNotification;
IJK_EXTERN NSString *const IJKMoviePlayerPlaybackStateDidChangeNotification;

IJK_EXTERN NSString *const IJKMoviePlayerIsAirPlayVideoActiveDidChangeNotification;
IJK_EXTERN NSString *const IJKMoviePlayerVideoDecoderOpenNotification;

IJK_EXTERN NSString *const IJKMoviePlayerFirstVideoFrameRenderedNotification;
IJK_EXTERN NSString *const IJKMoviePlayerFirstAudioFrameRenderedNotification;
@end

#pragma mark IJKMediaResource

@protocol IJKMediaSegmentResolver <NSObject>

- (NSString *)urlOfSegment:(int)segmentPosition;

@end

#pragma mark IJKMediaIoDelegate

/**
 * called before tcp connection
 *
 * @return
 *      original url:   continue connect.
 *      nil:            disconnect.
 *      new url:        use new url to connect.
 */
@protocol IJKMediaTcpOpenDelegate <NSObject>
- (NSString *)onTcpOpen:(int)streamIndex url:(NSString *)url;
@end

@protocol IJKMediaHttpOpenDelegate <NSObject>
- (NSString *)onHttpOpen:(int)streamIndex url:(NSString *)url;
@end

@protocol IJKMediaHttpRetryDelegate <NSObject>
- (NSString *)onHttpRetry:(int)streamIndex url:(NSString *)url retryCount:(int)retryCount;
@end

@protocol IJKMediaLiveRetryDelegate <NSObject>
- (NSString *)onLiveRetry:(int)streamIndex url:(NSString *)url retryCount:(int)retryCount;
@end
