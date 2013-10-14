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

@dynamic naturalSize;

@dynamic controlStyle;
@dynamic scalingMode;
@dynamic shouldAutoplay;
@dynamic useApplicationAudioSession;
@dynamic currentPlaybackRate;
@dynamic initialPlaybackTime;
@dynamic endPlaybackTime;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super initWithContentURL:aUrl];
    if (self) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;
        self.useApplicationAudioSession = NO;

        [self IJK_installMovieNotificationObservers];
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
}

- (void)IJK_removeMovieNotificationObservers
{
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];

    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerLoadStateDidChangeNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self];
    [[NSNotificationCenter defaultCenter]removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self];
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

@end
