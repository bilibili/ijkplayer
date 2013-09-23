//
//  IJKFFPlayerController.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-23.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKFFPlayerController.h"

@implementation IJKFFPlayerController {
    NSURL *_url;
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
    }
    return self;
}

- (void)prepareToPlay
{

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

@end
