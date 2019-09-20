/*
 * IJKSDLFboGLView.h
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2019 Befovy <befovy@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
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

#import <Foundation/Foundation.h>
#import "IJKSDLGLViewProtocol.h"

#include "ijksdl/ijksdl_vout.h"

NS_ASSUME_NONNULL_BEGIN

@interface IJKSDLFboGLView : NSObject<IJKSDLGLViewProtocol>

- (instancetype)initWithIJKCVPBViewProtocol:(id<IJKCVPBViewProtocol>) cvPBView;

- (void) display: (SDL_VoutOverlay *) overlay;

@end

NS_ASSUME_NONNULL_END
