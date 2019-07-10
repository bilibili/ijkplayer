/*
 * FIJKPlayer.h
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

#import <UIKit/UIKit.h>

//! Project version number for FIJKPlayer.
FOUNDATION_EXPORT double FIJKPlayerVersionNumber;

//! Project version string for FIJKPlayer.
FOUNDATION_EXPORT const unsigned char FIJKPlayerVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <FIJKPlayer/PublicHeader.h>
#import <FIJKPlayer/IJKMediaPlayback.h>
#import <FIJKPlayer/IJKMPMoviePlayerController.h>
#import <FIJKPlayer/IJKFFOptions.h>
#import <FIJKPlayer/IJKFFMediaPlayer.h>
#import <FIJKPlayer/IJKFFMoviePlayerController.h>
#import <FIJKPlayer/IJKAVMoviePlayerController.h>
#import <FIJKPlayer/IJKMediaModule.h>
#import <FIJKPlayer/IJKMediaPlayer.h>
#import <FIJKPlayer/IJKNotificationManager.h>
#import <FIJKPlayer/IJKKVOController.h>
#import <FIJKPlayer/IJKSDLGLViewProtocol.h>

// backward compatible for old names
#define IJKMediaPlaybackIsPreparedToPlayDidChangeNotification IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification
#define IJKMoviePlayerLoadStateDidChangeNotification IJKMPMoviePlayerLoadStateDidChangeNotification
#define IJKMoviePlayerPlaybackDidFinishNotification IJKMPMoviePlayerPlaybackDidFinishNotification
#define IJKMoviePlayerPlaybackDidFinishReasonUserInfoKey IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey
#define IJKMoviePlayerPlaybackStateDidChangeNotification IJKMPMoviePlayerPlaybackStateDidChangeNotification
#define IJKMoviePlayerIsAirPlayVideoActiveDidChangeNotification IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification
#define IJKMoviePlayerVideoDecoderOpenNotification IJKMPMoviePlayerVideoDecoderOpenNotification
#define IJKMoviePlayerFirstVideoFrameRenderedNotification IJKMPMoviePlayerFirstVideoFrameRenderedNotification
#define IJKMoviePlayerFirstAudioFrameRenderedNotification IJKMPMoviePlayerFirstAudioFrameRenderedNotification
#define IJKMoviePlayerFirstAudioFrameDecodedNotification IJKMPMoviePlayerFirstAudioFrameDecodedNotification
#define IJKMoviePlayerFirstVideoFrameDecodedNotification IJKMPMoviePlayerFirstVideoFrameDecodedNotification
#define IJKMoviePlayerOpenInputNotification IJKMPMoviePlayerOpenInputNotification
#define IJKMoviePlayerFindStreamInfoNotification IJKMPMoviePlayerFindStreamInfoNotification
#define IJKMoviePlayerComponentOpenNotification IJKMPMoviePlayerComponentOpenNotification

#define IJKMPMoviePlayerAccurateSeekCompleteNotification IJKMPMoviePlayerAccurateSeekCompleteNotification
#define IJKMoviePlayerSeekAudioStartNotification IJKMPMoviePlayerSeekAudioStartNotification
#define IJKMoviePlayerSeekVideoStartNotification IJKMPMoviePlayerSeekVideoStartNotification

