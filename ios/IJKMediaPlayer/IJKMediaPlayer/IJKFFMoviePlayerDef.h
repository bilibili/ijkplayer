/*
 * IJKFFMoviePlayerDef.h
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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
#include "ijkplayer/ios/ijkplayer_ios.h"
#include "ijkplayer/ijkmeta.h"


struct IJKSize {
    NSInteger width;
    NSInteger height;
};
typedef struct IJKSize IJKSize;

CG_INLINE IJKSize
IJKSizeMake(NSInteger width, NSInteger height)
{
    IJKSize size;
    size.width = width;
    size.height = height;
    return size;
}



struct IJKSampleAspectRatio {
    NSInteger numerator;
    NSInteger denominator;
};
typedef struct IJKSampleAspectRatio IJKSampleAspectRatio;

CG_INLINE IJKSampleAspectRatio
IJKSampleAspectRatioMake(NSInteger numerator, NSInteger denominator)
{
    IJKSampleAspectRatio sampleAspectRatio;
    sampleAspectRatio.numerator = numerator;
    sampleAspectRatio.denominator = denominator;
    return sampleAspectRatio;
}



@interface IJKFFMoviePlayerMessage : NSObject {
@public
    AVMessage _msg;
}
@end


@interface IJKFFMoviePlayerMessagePool : NSObject

- (IJKFFMoviePlayerMessagePool *)init;
- (IJKFFMoviePlayerMessage *) obtain;
- (void) recycle:(IJKFFMoviePlayerMessage *)msg;

@end
