//
//  IJKAudioKit.h
//  IJKMediaPlayer
//
//  Created by ZhangRui on 14-1-4.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol IJKAudioSessionDelegate <NSObject>

- (void)ijkAudioBeginInterruption;
- (void)ijkAudioEndInterruption;

@end

@interface IJKAudioKit : NSObject

+ (IJKAudioKit *)sharedInstance;
- (void)setupAudioSession:(id<IJKAudioSessionDelegate>) delegate;

@end
