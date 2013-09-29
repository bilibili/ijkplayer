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

@implementation IJKMPMoviePlayerController

@dynamic view;
@dynamic currentPlaybackTime;
@dynamic duration;
@dynamic playableDuration;

@dynamic isPreparedToPlay;
@dynamic playbackState;
@dynamic loadState;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super initWithContentURL:aUrl];
    if (self) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;
        self.useApplicationAudioSession = NO;
    }
    return self;
}

- (void)dealloc
{
    [self removeMovieNotificationObservers];
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

#pragma mark Movie Notification Handlers

/* Register observers for the various movie object notifications. */
-(void)installMovieNotificationObservers
{
	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:)
                                                 name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(dispatchMPMoviePlayerLoadStateDidChangeNotification:)
                                                 name:MPMoviePlayerLoadStateDidChangeNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(dispatchMPMoviePlayerPlaybackDidFinishNotification:)
                                                 name:MPMoviePlayerPlaybackDidFinishNotification
                                               object:self];

	[[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(dispatchMPMoviePlayerPlaybackStateDidChangeNotification:)
                                                 name:MPMoviePlayerPlaybackStateDidChangeNotification
                                               object:self];
}

- (void)removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];

    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self];
}

- (void)dispatchMPMediaPlaybackIsPreparedToPlayDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)dispatchMPMoviePlayerLoadStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerLoadStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

- (void)dispatchMPMoviePlayerPlaybackDidFinishNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification object:notification.object userInfo:notification.userInfo];
}

- (void)dispatchMPMoviePlayerPlaybackStateDidChangeNotification:(NSNotification*)notification
{
    [[NSNotificationCenter defaultCenter] postNotificationName:IJKMoviePlayerPlaybackStateDidChangeNotification object:notification.object userInfo:notification.userInfo];
}

@end
