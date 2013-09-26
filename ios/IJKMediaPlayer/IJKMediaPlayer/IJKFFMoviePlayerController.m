/*
 * IJKFFPlayerController.m
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

#import "IJKFFPlayerController.h"
#import "IJKFFPlayerDef.h"
#import "IJKMediaPlayback.h"


@implementation IJKFFPlayerController {
    NSURL *_url;
    IjkMediaPlayer *_mediaPlayer;
    IJKFFPlayerMessagePool *_msgPool;
}

@synthesize view = _view;
@synthesize currentPlaybackTime;
@synthesize duration;
@synthesize playableDuration;

@synthesize playbackDelegate;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super init];
    if (self) {
        ijkmp_global_init();

        _url = aUrl;
        _mediaPlayer = ijkmp_ios_create(media_player_msg_loop);
        _msgPool = [[IJKFFPlayerMessagePool alloc] init];

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

- (void)postEvent: (IJKFFPlayerMessage *)msg
{
    if (!msg)
        return;

    id<IJKMediaPlaybackDelegate> delegate = self.playbackDelegate;
    if (!delegate)
        return;

    AVMessage *avmsg = &msg->_msg;
    switch (avmsg->what) {
        case FFP_MSG_FLUSH:
            break;
        case FFP_MSG_ERROR:
            NSLog(@"FFP_MSG_ERROR: %d", avmsg->arg1);
            if ([delegate respondsToSelector:@selector(playerDidFail:)])
                [delegate playerDidFail:avmsg->arg1];
            break;
        case FFP_MSG_PREPARED:
            NSLog(@"FFP_MSG_PREPARED:");
            if ([delegate respondsToSelector:@selector(playerDidPrepare:)])
                [delegate playerDidPrepare];
            break;
        case FFP_MSG_COMPLETED:
            NSLog(@"FFP_MSG_COMPLETED:");
            if ([delegate respondsToSelector:@selector(playerDidComplete:)])
                [delegate playerDidComplete];
            break;
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            NSLog(@"FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if ([delegate respondsToSelector:@selector(playerDidChangeVideoSize:)])
                [delegate playerDidChangeVideoSize:IJKSizeMake(avmsg->arg1, avmsg->arg2)];
            break;
        case FFP_MSG_SAR_CHANGED:
            NSLog(@"FFP_MSG_SAR_CHANGED: %d, %d", avmsg->arg1, avmsg->arg2);
            if ([delegate respondsToSelector:@selector(playerDidChangeSampleAspectRatio:)])
                [delegate playerDidChangeSampleAspectRatio:IJKSampleAspectRatioMake(avmsg->arg1, avmsg->arg2)];
            break;
        case FFP_MSG_BUFFERING_START:
            NSLog(@"FFP_MSG_BUFFERING_START:");
            if ([delegate respondsToSelector:@selector(playerDidStartBuffering:)])
                [delegate playerDidStartBuffering];
            break;
        case FFP_MSG_BUFFERING_END:
            NSLog(@"FFP_MSG_BUFFERING_END:");
            if ([delegate respondsToSelector:@selector(playerDidStopBuffering:)])
                [delegate playerDidStopBuffering];
            break;
        case FFP_MSG_BUFFERING_UPDATE:
            NSLog(@"FFP_MSG_BUFFERING_UPDATE: %d, %d", avmsg->arg1, avmsg->arg2);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            NSLog(@"FFP_MSG_BUFFERING_BYTES_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            NSLog(@"FFP_MSG_BUFFERING_TIME_UPDATE: %d", avmsg->arg1);
            break;
        case FFP_MSG_SEEK_COMPLETE:
            NSLog(@"FFP_MSG_SEEK_COMPLETE:");
            if ([delegate respondsToSelector:@selector(playerDidSeek:)])
                [delegate playerDidSeek];
            break;
        default:
            NSLog(@"unknown FFP_MSG_xxx(%d)", avmsg->what);
            break;
    }

    [_msgPool recycle:msg];
}

- (IJKFFPlayerMessage *) obtainMessage {
    return [_msgPool obtain];
}

inline static IJKFFPlayerController *ffplayerRetain(void *arg) {
    return (__bridge_transfer IJKFFPlayerController *) arg;
}

int media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
        __weak IJKFFPlayerController *ffpController = ffplayerRetain(ijkmp_set_weak_thiz(mp, NULL));

        while (ffpController && true) {
            @autoreleasepool {
                IJKFFPlayerMessage *msg = [ffpController obtainMessage];
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
