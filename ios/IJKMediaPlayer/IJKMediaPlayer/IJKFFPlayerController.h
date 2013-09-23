//
//  IJKFFPlayerController.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-23.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMediaPlayback.h"

@interface IJKFFPlayerController : NSObject <IJKMediaPlayback>

- (id)initWithContentURL:(NSURL *)aUrl;

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;
- (BOOL)isPlaying;

@end
