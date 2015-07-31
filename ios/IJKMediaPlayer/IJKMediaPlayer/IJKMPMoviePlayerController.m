/*
 * IJKMPMoviePlayerController.m
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

#import "IJKMPMoviePlayerController.h"
#import "IJKAudioKit.h"

@implementation IJKMPMoviePlayerController

@dynamic view;
@dynamic currentPlaybackTime;
@dynamic duration;
@dynamic playableDuration;
@synthesize bufferingProgress = _bufferingProgress;

@dynamic isPreparedToPlay;
@dynamic playbackState;
@dynamic loadState;

@dynamic controlStyle;
@dynamic scalingMode;
@dynamic shouldAutoplay;
@synthesize isDanmakuMediaAirPlay = _isDanmakuMediaAirPlay;

@synthesize numberOfBytesTransferred = _numberOfBytesTransferred;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super initWithContentURL:aUrl];
    if (self) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;
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
    return [super thumbnailImageAtTime:self.currentPlaybackTime timeOption:MPMovieTimeOptionExact];
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
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:nil userInfo:nil];
}

#pragma mark Movie Notification Handlers

/* Register observers for the various movie object notifications. */
-(void)IJK_installMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(IJK_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:)
                                                 name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(IJK_dispatchMPMoviePlayerLoadStateDidChangeNotification:)
                                                 name:MPMoviePlayerLoadStateDidChangeNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(IJK_dispatchMPMoviePlayerPlaybackDidFinishNotification:)
                                                 name:MPMoviePlayerPlaybackDidFinishNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(IJK_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:)
                                                 name:MPMoviePlayerPlaybackStateDidChangeNotification
                                               object:self];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(IJK_dispatchMoviePlayerIsAirPlayVideoActiveDidChangeNotification:)
                                                 name:MPMoviePlayerIsAirPlayVideoActiveDidChangeNotification
                                               object:self];
}

- (void)IJK_removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:self];
}

- (void)IJK_dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerLoadStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerLoadStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerPlaybackDidFinishNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMPMoviePlayerPlaybackStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerPlaybackStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)IJK_dispatchMoviePlayerIsAirPlayVideoActiveDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerIsAirPlayVideoActiveDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)setPauseInBackground:(BOOL)pause
{
    //mpPlayer还未找到方法实现
}

@end
