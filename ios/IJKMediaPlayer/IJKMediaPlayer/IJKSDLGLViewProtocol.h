/*
 * IJKSDLGLViewProtocol.h
 *
 * Copyright (c) 2017 Bilibili
 * Copyright (c) 2017 raymond <raymondzheng1412@gmail.com>
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

#ifndef IJKSDLGLViewProtocol_h
#define IJKSDLGLViewProtocol_h

#import <UIKit/UIKit.h>

typedef struct IJKOverlay IJKOverlay;
struct IJKOverlay {
    int w;
    int h;
    UInt32 format;
    int planes;
    UInt16 *pitches;
    UInt8 **pixels;
    int sar_num;
    int sar_den;
    CVPixelBufferRef pixel_buffer;
};

/// A protocol to guarantee that non-thread safe values are accessed from
/// an object that can guarantee thread safety.
/// IJKFFMovieController and IJKSDLGLView will access non-thread safe
/// values from a background thread. The object that conforms to this protocol
/// should safely handle the access of these properties, internally setting them on main thread.
@protocol IJKThreadSafeMainScreen

/// Returns the current bounds of the screen
@property (readonly) CGRect bounds;

/// Returns the current scale of the screen
@property (readonly) CGFloat scale;

@end

/// A protocol to guarantee that non-thread safe values are accessed from
/// an object that can guarantee thread safety.
/// IJKFFMovieController and IJKSDLGLView will access non-thread safe
/// values from a background thread. The object that conforms to this protocol
/// should safely handle the access of these properties, internally setting them on main thread.
@protocol IJKThreadSafeApplicationState

/// Returns the current state of the application
@property (readonly) UIApplicationState applicationState;

@end


@protocol IJKSDLGLViewProtocol <NSObject>
- (UIImage*) snapshot;
@property(nonatomic, readonly) CGFloat  fps;
@property(nonatomic)        CGFloat  scaleFactor;
@property(nonatomic)        BOOL  isThirdGLView;
- (void) display_pixels: (IJKOverlay *) overlay;
@end

#endif /* IJKSDLGLViewProtocol_h */
