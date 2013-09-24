//
//  IJKFFPlayerController.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-23.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKFFPlayerController.h"
#include "ijkplayer/ios/ijkplayer_ios.h"

@implementation IJKFFPlayerController {
    NSURL *_url;

    __weak IJKFFPlayerController *_weakSelf;

    IjkMediaPlayer *_mediaPlayer;
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
        _weakSelf = self;
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

    ijkmp_seek_to(_mediaPlayer, aCurrentPlaybackTime);
}

- (NSTimeInterval)getCurrentPlaybackTime
{
    if (!_mediaPlayer)
        return 0.0f;

    return ijkmp_get_current_position(_mediaPlayer);
}

- (NSTimeInterval)getDuration
{
    if (!_mediaPlayer)
        return 0.0f;

    return ijkmp_get_duration(_mediaPlayer);
}

- (void)postEvent: (AVMessage *)msg
{
    
}

int media_player_msg_loop(void* arg)
{
    @autoreleasepool {
        IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
        IJKFFPlayerController *ffpController = (__bridge_transfer IJKFFPlayerController *) ijkmp_set_weak_thiz(mp, NULL);

        __weak IJKFFPlayerController *weakSelf = ffpController->_weakSelf;
        ffpController = nil;

        while (weakSelf && true) {
            AVMessage msg;

            int retval = ijkmp_get_msg(mp, &msg, 1);
            if (retval < 0)
                break;

            // block-get should never return 0
            assert(retval > 0);
            @autoreleasepool {
                [ffpController postEvent:&msg];
            }
        }

        return 0;
    }
}

@end
