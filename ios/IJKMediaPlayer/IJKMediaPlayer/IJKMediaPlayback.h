//
//  IJKMediaPlayback.h
//  IJKMediaPlayback
//
//  Created by ZhangRui on 13-9-19.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

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
