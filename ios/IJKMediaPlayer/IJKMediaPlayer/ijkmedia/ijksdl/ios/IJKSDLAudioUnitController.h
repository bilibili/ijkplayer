//
//  IJKSDLAudioUnitController.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-23.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ijksdl/ijksdl_aout.h"

@interface IJKSDLAudioUnitController : NSObject

- (id)initWithAudioSpec:(SDL_AudioSpec *)aSpec;

- (void)play;
- (void)pause;
- (void)flush;
- (void)stop;
- (void)close;

@property (nonatomic, readonly) SDL_AudioSpec spec;
@property (nonatomic, getter=isPaused) BOOL paused;

@end
