/*
 * IJKMediaPlayback.m
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

#import "IJKMediaPlayback.h"

NSString *const IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification = @"IJKMPMediaPlaybackIsPreparedToPlayDidChangeNotification";

NSString *const IJKMPMoviePlayerPlaybackDidFinishNotification = @"IJKMPMoviePlayerPlaybackDidFinishNotification";
NSString *const IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey =
    @"IJKMPMoviePlayerPlaybackDidFinishReasonUserInfoKey";
NSString *const IJKMPMoviePlayerPlaybackStateDidChangeNotification = @"IJKMPMoviePlayerPlaybackStateDidChangeNotification";
NSString *const IJKMPMoviePlayerLoadStateDidChangeNotification = @"IJKMPMoviePlayerLoadStateDidChangeNotification";

NSString *const IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification = @"IJKMPMoviePlayerIsAirPlayVideoActiveDidChangeNotification";

NSString *const IJKMPMovieNaturalSizeAvailableNotification = @"IJKMPMovieNaturalSizeAvailableNotification";

NSString *const IJKMPMoviePlayerVideoDecoderOpenNotification = @"IJKMPMoviePlayerVideoDecoderOpenNotification";

NSString *const IJKMPMoviePlayerFirstVideoFrameRenderedNotification = @"IJKMPMoviePlayerFirstVideoFrameRenderedNotification";
NSString *const IJKMPMoviePlayerFirstAudioFrameRenderedNotification = @"IJKMPMoviePlayerFirstAudioFrameRenderedNotification";
NSString *const IJKMPMoviePlayerFirstAudioFrameDecodedNotification  = @"IJKMPMoviePlayerFirstAudioFrameDecodedNotification";
NSString *const IJKMPMoviePlayerFirstVideoFrameDecodedNotification  = @"IJKMPMoviePlayerFirstVideoFrameDecodedNotification";
NSString *const IJKMPMoviePlayerOpenInputNotification               = @"IJKMPMoviePlayerOpenInputNotification";
NSString *const IJKMPMoviePlayerFindStreamInfoNotification          = @"IJKMPMoviePlayerFindStreamInfoNotification";
NSString *const IJKMPMoviePlayerComponentOpenNotification           = @"IJKMPMoviePlayerComponentOpenNotification";

NSString *const IJKMPMoviePlayerAccurateSeekCompleteNotification = @"IJKMPMoviePlayerAccurateSeekCompleteNotification";

NSString *const IJKMPMoviePlayerDidSeekCompleteNotification = @"IJKMPMoviePlayerDidSeekCompleteNotification";
NSString *const IJKMPMoviePlayerDidSeekCompleteTargetKey = @"IJKMPMoviePlayerDidSeekCompleteTargetKey";
NSString *const IJKMPMoviePlayerDidSeekCompleteErrorKey = @"IJKMPMoviePlayerDidSeekCompleteErrorKey";
NSString *const IJKMPMoviePlayerDidAccurateSeekCompleteCurPos = @"IJKMPMoviePlayerDidAccurateSeekCompleteCurPos";

NSString *const IJKMPMoviePlayerSeekAudioStartNotification  = @"IJKMPMoviePlayerSeekAudioStartNotification";
NSString *const IJKMPMoviePlayerSeekVideoStartNotification  = @"IJKMPMoviePlayerSeekVideoStartNotification";

@implementation IJKMediaUrlOpenData {
    NSString *_url;
    BOOL _handled;
    BOOL _urlChanged;
}

- (id)initWithUrl:(NSString *)url
            event:(IJKMediaEvent)event
     segmentIndex:(int)segmentIndex
     retryCounter:(int)retryCounter
{
    self = [super init];
    if (self) {
        self->_url          = url;
        self->_event        = event;
        self->_segmentIndex = segmentIndex;
        self->_retryCounter = retryCounter;

        self->_error        = 0;
        self->_handled      = NO;
        self->_urlChanged   = NO;
    }
    return self;
}

- (void)setHandled:(BOOL)handled
{
    _handled = handled;
}

- (BOOL)isHandled
{
    return _handled;
}

- (BOOL)isUrlChanged
{
    return _urlChanged;
}

- (NSString *)url
{
    return _url;
}

- (void)setUrl:(NSString *)url
{
    assert(url);

    _handled = YES;

    if (![self.url isEqualToString:url]) {
        _urlChanged = YES;
        _url = url;
    }
}

@end
