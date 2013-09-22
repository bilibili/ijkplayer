//
//  IJKMediaPlayback.h
//  IJKMediaPlayback
//
//  Created by ZhangRui on 13-9-19.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MediaPlayer/MediaPlayer.h>


@protocol IJKMediaPlayback;


#pragma mark IJKMediaPlaybackDelegate

@protocol IJKMediaPlaybackDelegate <NSObject>

- (void)onEvent:(id<IJKMediaPlayback>)player;

@end


#pragma mark IJKMediaPlayback

@protocol IJKMediaPlayback <NSObject, MPMediaPlayback>

- (void)prepareToPlay;
- (void)play;
- (void)pause;
- (void)stop;

@property(nonatomic, readonly)  UIView *view;
@property(nonatomic, readonly)  BOOL isPreparedToPlay;
@property(nonatomic)            NSTimeInterval currentPlaybackTime;
@property(nonatomic)            float currentPlaybackRate;
@property(nonatomic, weak)      id<IJKMediaPlaybackDelegate> delegate;

@end
