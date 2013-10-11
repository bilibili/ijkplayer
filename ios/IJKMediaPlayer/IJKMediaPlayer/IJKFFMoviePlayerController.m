/*
 * IJKFFMoviePlayerController.m
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

#import "IJKFFMoviePlayerController.h"
#import "IJKFFMoviePlayerDef.h"
#import "IJKMediaPlayback.h"

@implementation IJKFFMoviePlayerController {
    NSURL *_url;
    IjkMediaPlayer *_mediaPlayer;
    IJKFFMoviePlayerMessagePool *_msgPool;

    NSInteger _videoWidth;
    NSInteger _videoHeight;
    NSInteger _sampleAspectRatioNumerator;
    NSInteger _sampleAspectRatioDenominator;

    NSInteger _bufferingTime;
}

@synthesize view = _view;
@synthesize currentPlaybackTime;
@synthesize duration;
@synthesize playableDuration;

@synthesize isPreparedToPlay;
@synthesize playbackState;
@synthesize loadState = _loadState;

@synthesize naturalSize = _naturalSize;

@synthesize controlStyle = _controlStyle;
@synthesize scalingMode = _scalingMode;
@synthesize shouldAutoplay = _shouldAutoplay;
@synthesize useApplicationAudioSession = _useApplicationAudioSession;
@synthesize currentPlaybackRate = _currentPlaybackRate;
@synthesize initialPlaybackTime = _initialPlaybackTime;
@synthesize endPlaybackTime = _endPlaybackTime;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super init];
    if (self) {
        ijkmp_global_init();

        _controlStyle = MPMovieControlStyleNone;
        _scalingMode = MPMovieScalingModeAspectFit;
        _shouldAutoplay = NO;
        _useApplicationAudioSession = NO;
        _currentPlaybackRate = 1.0f;
        _initialPlaybackTime = 0;
        _endPlaybackTime = 0;

        _url = aUrl;
        _mediaPlayer = ijkmp_ios_create(media_player_msg_loop);
        _msgPool = [[IJKFFMoviePlayerMessagePool alloc] init];

        ijkmp_set_weak_thiz(_mediaPlayer, (__bridge_retained void *) self);

        IJKSDLGLView *glView = [[IJKSDLGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
        self->_view = glView;

        ijkmp_ios_set_glview(_mediaPlayer, glView);
        ijkmp_set_overlay_format(_mediaPlayer, SDL_FCC_I420);
    }
    return self;
}

- (void)prepareToPlay
{
    if (!_mediaPlayer)
        return;

    ijkmp_set_data_source(_mediaPlayer, [[_url absoluteString] UTF8String]);
    ijkmp_set_format_option(_mediaPlayer, "safe", "0"); // for concat demuxer
    ijkmp_prepare_async(_mediaPlayer);
}

- (void)play
{
    if (!_mediaPlayer)
        return;

    ijkmp_start(_mediaPlayer);
}

- (void)pause
{
    if (!_mediaPlayer)
        return;

    ijkmp_pause(_mediaPlayer);
}

- (void)stop
{
    if (!_mediaPlayer)
        return;

    ijkmp_stop(_mediaPlayer);
}

- (BOOL)isPlaying
{
    if (!_mediaPlayer)
        return NO;

    return ijkmp_is_playing(_mediaPlayer);
}

- (MPMoviePlaybackState)playbackState
{
    if (!_mediaPlayer)
        return NO;

    MPMoviePlaybackState mp_state = MPMoviePlaybackStateStopped;
    int state = ijkmp_get_state(_mediaPlayer);
    switch (state) {
        case MP_STATE_STOPPED:
        case MP_STATE_COMPLETED:
        case MP_STATE_ERROR:
        case MP_STATE_END:
            mp_state = MPMoviePlaybackStateStopped;
            break;
        case MP_STATE_IDLE:
        case MP_STATE_INITIALIZED:
        case MP_STATE_ASYNC_PREPARING:
        case MP_STATE_PREPARED:
        case MP_STATE_PAUSED:
            mp_state = MPMoviePlaybackStatePaused;
            break;
        case MP_STATE_STARTED:
            mp_state = MPMoviePlaybackStatePlaying;
            break;
    }
    // MPMoviePlaybackStatePlaying,
    // MPMoviePlaybackStatePaused,
    // MPMoviePlaybackStateStopped,
    // MPMoviePlaybackStateInterrupted,
    // MPMoviePlaybackStateSeekingForward,
    // MPMoviePlaybackStateSeekingBackward
    return ijkmp_is_playing(_mediaPlayer);
}

- (void)setCurrentPlaybackTime:(NSTimeInterval)aCurrentPlaybackTime
{
    if (!_mediaPlayer)
        return;

    ijkmp_seek_to(_mediaPlayer, aCurrentPlaybackTime * 1000);
}

- (NSTimeInterval)currentPlaybackTime
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = ijkmp_get_current_position(_mediaPlayer);
    return ret / 1000;
}

- (NSTimeInterval)duration
{
    if (!_mediaPlayer)
        return 0.0f;

    NSTimeInterval ret = ijkmp_get_duration(_mediaPlayer);
    return ret / 1000;
}

- (NSTimeInterval)playableDuration
{
    return self.currentPlaybackTime + ((NSTimeInterval)_bufferingTime) / 1000;
}

// deprecated, for MPMoviePlayerController compatiable
- (UIImage *)thumbnailImageAtTime:(NSTimeInterval)playbackTime timeOption:(MPMovieTimeOption)option
{
    return nil;
}

- (void)postEvent: (IJKFFMoviePlayerMessage *)msg
{
    if (!msg)
        return;

    AVMessage *avmsg = &msg->_msg;
    switch (avmsg->what) {
        case FFP_MSG_FLUSH:
            break;
        case FFP_MSG_ERROR: {
            NSLog(@"FFP_MSG_ERROR: %d", avmsg->arg1);

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification object:self];

            [[NSNotificationCenter defaultCenter]
                postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification
                object:self
                userInfo:@{
                    MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackError),
                    @"error": @(avmsg->arg1)}];
            break;
        }
        case FFP_MSG_PREPARED:
            NSLog(@"FFP_MSG_PREPARED:");

            isPreparedToPlay = YES;

            [[NSNotificationCenter defaultCenter] postNotificationName:IJKMediaPlaybackIsPreparedToPlayDidChangeNotification object:self];

            break;
        case FFP_MSG_COMPLETED: {

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification object:self];

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerPlaybackDidFinishNotification
             object:self
             userInfo:@{MPMoviePlayerPlaybackDidFinishReasonUserInfoKey: @(MPMovieFinishReasonPlaybackError)}];
            break;
        }
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            NSLog(@"FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _videoWidth = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _videoHeight = avmsg->arg2;
            // TODO: notify size changed
            break;
        case FFP_MSG_SAR_CHANGED:
            NSLog(@"FFP_MSG_SAR_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if (avmsg->arg1 > 0)
                _sampleAspectRatioNumerator = avmsg->arg1;
            if (avmsg->arg2 > 0)
                _sampleAspectRatioDenominator = avmsg->arg2;
            break;
        case FFP_MSG_BUFFERING_START: {
            NSLog(@"FFP_MSG_BUFFERING_START:");

            _loadState = MPMovieLoadStateStalled;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerLoadStateDidChangeNotification
             object:self];
            break;
        }
        case FFP_MSG_BUFFERING_END: {
            NSLog(@"FFP_MSG_BUFFERING_END:");

            _loadState = MPMovieLoadStatePlaythroughOK;

            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerLoadStateDidChangeNotification
             object:self];
            break;
        }
        case FFP_MSG_BUFFERING_UPDATE:
            // NSLog(@"FFP_MSG_BUFFERING_UPDATE: %d, %d", avmsg->arg1, avmsg->arg2);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            // NSLog(@"FFP_MSG_BUFFERING_BYTES_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            _bufferingTime = avmsg->arg1;
            // NSLog(@"FFP_MSG_BUFFERING_TIME_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_PLAYBACK_STATE_CHANGED:
            [[NSNotificationCenter defaultCenter]
             postNotificationName:IJKMoviePlayerPlaybackStateDidChangeNotification
             object:self];
            break;
        case FFP_MSG_SEEK_COMPLETE:
            // NSLog(@"FFP_MSG_SEEK_COMPLETE:");
            break;
        default:
            // NSLog(@"unknown FFP_MSG_xxx(%d)", avmsg->what);
            break;
    }

    [_msgPool recycle:msg];
}

- (IJKFFMoviePlayerMessage *) obtainMessage {
    return [_msgPool obtain];
}

inline static IJKFFMoviePlayerController *ffplayerRetain(void *arg) {
    return (__bridge_transfer IJKFFMoviePlayerController *) arg;
}

int media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
        __weak IJKFFMoviePlayerController *ffpController = ffplayerRetain(ijkmp_set_weak_thiz(mp, NULL));

        while (ffpController) {
            @autoreleasepool {
                IJKFFMoviePlayerMessage *msg = [ffpController obtainMessage];
                if (!msg)
                    break;

                int retval = ijkmp_get_msg(mp, &msg->_msg, 1);
                if (retval < 0)
                    break;

                // block-get should never return 0
                assert(retval > 0);

                [ffpController performSelectorOnMainThread:@selector(postEvent:) withObject:msg waitUntilDone:NO];
            }
        }

        return 0;
    }
}

@end
