/*
 * IJKMediaPlayback.h
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
#import <UIKit/UIKit.h>


@protocol IJKMediaPlayback;



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


#pragma mark IJKMediaPlaybackDelegate

@protocol IJKMediaPlaybackDelegate <NSObject>

@optional
- (void)playerDidFail:(NSInteger)error;
- (void)playerDidPrepare;
- (void)playerDidComplete;
- (void)playerDidChangeVideoSize:(IJKSize)size;
- (void)playerDidChangeSampleAspectRatio:(IJKSampleAspectRatio)sampleAspectRatio;
- (void)playerDidStartBuffering;
- (void)playerDidStopBuffering;
- (void)playerDidSeek;

@end


#pragma mark IJKMediaPlayback

@protocol IJKMediaPlayback <NSObject>

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;
- (BOOL)isPlaying;

@property(nonatomic, readonly)  UIView *view;
@property(nonatomic)            NSTimeInterval currentPlaybackTime;
@property(nonatomic, readonly)  NSTimeInterval duration;
@property(nonatomic, readonly)  NSTimeInterval playableDuration;

@property(nonatomic, weak)      id<IJKMediaPlaybackDelegate> playbackDelegate;

@end
