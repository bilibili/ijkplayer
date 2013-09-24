//
//  IJKSDLGLRender.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "ijksdl/ijksdl_vout.h"

@protocol IJKSDLGLRender
- (BOOL) isValid;
- (NSString *) fragmentShader;
- (void) resolveUniforms: (GLuint) program;
- (void) display: (SDL_VoutOverlay *) overlay;
- (BOOL) prepareDisplay;
@end
