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


#pragma mark IJKMediaPlaybackDelegate

@protocol IJKMediaPlaybackDelegate <NSObject>

- (void)onEvent:(id<IJKMediaPlayback>)player;

@end


#pragma mark IJKMediaPlayback

@protocol IJKMediaPlayback <NSObject>

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;

@property(nonatomic, readonly)  UIView *view;
@property(nonatomic, readonly)  BOOL isPreparedToPlay;
@property(nonatomic)            NSTimeInterval currentPlaybackTime;
@property(nonatomic, readonly)  NSTimeInterval duration;
@property(nonatomic, readonly)  NSTimeInterval playableDuration;

@property(nonatomic, weak)      id<IJKMediaPlaybackDelegate> playbackDelegate;

@end
