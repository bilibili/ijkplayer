//
//  IJKMPMoviePlayerController.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-22.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMPMoviePlayerController.h"

@implementation IJKMPMoviePlayerController

@dynamic view;
@dynamic currentPlaybackTime;
@dynamic duration;
@dynamic playableDuration;

@synthesize playbackDelegate;

- (id)initWithContentURL:(NSURL *)url
{
    self = [super initWithContentURL:url];
    if (self) {
        self.controlStyle = MPMovieControlStyleNone;
        self.scalingMode = MPMovieScalingModeAspectFit;
        self.shouldAutoplay = YES;
        self.useApplicationAudioSession = NO;
    }
    return self;
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

@end
