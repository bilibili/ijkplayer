/*
 * IJKMPMoviePlayerController.m
 *
 * Copyright (c) 2013 Bilibili
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

#import "IJKMPMoviePlayerController.h"
#import "IJKAudioKit.h"
#import "IJKNotificationManager.h"

@implementation IJKMPMoviePlayerController
{
    IJKNotificationManager *_notificationManager;
}

@dynamic view;
@dynamic currentPlaybackTime;
@dynamic duration;
@dynamic playableDuration;
@synthesize bufferingProgress = _bufferingProgress;

@dynamic isPreparedToPlay;
@dynamic playbackState;
@dynamic loadState;

@dynamic naturalSize;
@dynamic scalingMode;
@dynamic shouldAutoplay;
@synthesize isDanmakuMediaAirPlay = _isDanmakuMediaAirPlay;

@synthesize numberOfBytesTransferred = _numberOfBytesTransferred;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super initWithContentURL:aUrl];
    if (self) {
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;

        _notificationManager = [[IJKNotificationManager alloc] init];
        [self IJK_installMovieNotificationObservers];

        [[IJKAudioKit sharedInstance] setupAudioSession];
        
        _bufferingProgress = -1;
    }
    return self;
}

- (id)initWithContentURLString:(NSString *)aUrl
{
    NSURL *url;
    if ([aUrl rangeOfString:@"/"].location == 0) {
        //本地
        url = [NSURL fileURLWithPath:aUrl];
    }
    else {
        url = [NSURL URLWithString:aUrl];
    }
    
    self = [self initWithContentURL:url];
    if (self) {
        
    }
    return self;
}

- (void)dealloc
{
    [self IJK_removeMovieNotificationObservers];
}

- (BOOL)isPlaying
{
    switch (self.playbackState) {
        case MPMoviePlaybackStatePlaying:
            return YES;
        default:
            return NO;
    }
}

- (void)shutdown
{
    // do nothing
}

-(int64_t)numberOfBytesTransferred
{
    NSArray *events = self.accessLog.events;
    if (events.count>0) {
        MPMovieAccessLogEvent *currentEvent = [events objectAtIndex:events.count -1];
        return currentEvent.numberOfBytesTransferred;
    }
    return 0;
}

- (UIImage *)thumbnailImageAtCurrentTime
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    return [super thumbnailImageAtTime:self.currentPlaybackTime timeOption:MPMovieTimeOptionExact];
#pragma clang diagnostic pop
}

-(BOOL)allowsMediaAirPlay
{
    if (!self)
        return NO;
    return [self allowsAirPlay];
}

-(void)setAllowsMediaAirPlay:(BOOL)b
{
    if (!self)
        return;
    [self setAllowsAirPlay:b];
}

-(BOOL)airPlayMediaActive
{
    if (!self)
        return NO;
    
    return self.airPlayVideoActive || self.isDanmakuMediaAirPlay;
}

-(BOOL)isDanmakuMediaAirPlay
{
    return _isDanmakuMediaAirPlay;
}

-(void)setIsDanmakuMediaAirPlay:(BOOL)isDanmakuMediaAirPlay
{
    _isDanmakuMediaAirPlay = isDanmakuMediaAirPlay;
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:nil userInfo:nil];
}

-(void)setPlaybackRate:(float)playbackRate
{
    NSLog(@"[MPMoviePlayerController setPlaybackRate] is not supported\n");
}

-(float)playbackRate
{
    return 1.0f;
}

-(void)setPlaybackVolume:(float)playbackVolume
{
    NSLog(@"[MPMoviePlayerController setPlaybackVolume] is not supported\n");
}

-(float)playbackVolume
{
    return 1.0f;
}

#pragma mark Movie Notification Handlers

/* Register observers for the various movie object notifications. */
-(void)IJK_installMovieNotificationObservers
{
    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:)
                                 name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification
                               object:self];

    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMPMoviePlayerLoadStateDidChangeNotification:)
                                 name:MPMoviePlayerLoadStateDidChangeNotification
                               object:self];

    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMPMoviePlayerPlaybackDidFinishNotification:)
                                 name:MPMoviePlayerPlaybackDidFinishNotification
                               object:self];

    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:)
                                 name:MPMoviePlayerPlaybackStateDidChangeNotification
                               object:self];

    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMoviePlayerIsAirPlayVideoActiveDidChangeNotification:)
                                 name:MPMoviePlayerIsAirPlayVideoActiveDidChangeNotification
                               object:self];
    [_notificationManager addObserver:self
                             selector:@selector(IJK_dispatchMoviePlayerNaturalSizeAvailableNotification:)
                                 name:MPMovieNaturalSizeAvailableNotification
                               object:self];
}

- (void)IJK_removeMovieNotificationObservers
{
    [_notificationManager removeAllObservers:self];
}

- (void)IJK_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerLoadStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerLoadStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerPlaybackDidFinishNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerPlaybackDidFinishNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerPlaybackStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMoviePlayerIsAirPlayVideoActiveDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMoviePlayerNaturalSizeAvailableNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMPMovieNaturalSizeAvailableNotification object:notification.object userInfo:notification.userInfo];
}

- (void)setPauseInBackground:(BOOL)pause
{
    //mpPlayer还未找到方法实现
}

@end
