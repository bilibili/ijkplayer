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

    IjkMediaPlayer *_mediaPlayer;
}

@synthesize view;
@synthesize currentPlaybackTime;
@synthesize duration;
@synthesize playableDuration;

@synthesize playbackDelegate;

- (id)initWithContentURL:(NSURL *)aUrl
{
    self = [super init];
    if (self) {
        self->_url = aUrl;
        self->_mediaPlayer = ijkmp_ios_create(media_player_msg_loop);
        ijkmp_set_weak_thiz(self->_mediaPlayer, (__bridge_retained void *) self);
    }
    return self;
}

- (void)prepareToPlay
{
    ijkmp_set_data_source(_mediaPlayer, [[self->_url absoluteString] UTF8String]);
}

- (void)play
{

}

- (void)pause
{

}

- (void)stop
{

}

- (BOOL)isPlaying
{
    return NO;
}

- (void)postEvent: (AVMessage *)msg
{
    
}

void *media_player_msg_loop(void* arg)
{
    IjkMediaPlayer *mp = (IjkMediaPlayer*)arg;
    IJKFFPlayerController *ffpController = (__bridge_transfer IJKFFPlayerController *) ijkmp_set_weak_thiz(mp, NULL);

    while (true) {
        AVMessage msg;

        int retval = ijkmp_get_msg(mp, &msg, 1);
        if (retval < 0)
            break;

        // block-get should never return 0
        assert(retval > 0);
        [ffpController postEvent:&msg];
    }

    return NULL;
}

@end
