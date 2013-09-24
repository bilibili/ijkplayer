//
//  IJKSDLGLView.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-24.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <UIKit/UIKit.h>

#include "ijksdl/ijksdl_vout.h"

@interface IJKSDLGLView : UIView

- (id) initWithFrame:(CGRect)frame;

- (void) display: (SDL_VoutOverlay *) overlay;

@end
