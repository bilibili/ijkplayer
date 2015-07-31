//
//  IJKMediaModule.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 14-3-14.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

#import "IJKMediaModule.h"
#import <UIKit/UIKit.h>

@implementation IJKMediaModule

@synthesize appIdleTimerDisabled         = _appIdleTimerDisabled;
@synthesize mediaModuleIdleTimerDisabled = _mediaModuleIdleTimerDisabled;

+ (IJKMediaModule *)sharedModule
{
    static IJKMediaModule *obj = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        obj = [[IJKMediaModule alloc] init];
    });
    return obj;
}

- (void)setAppIdleTimerDisabled:(BOOL) idleTimerDisabled
{
    _appIdleTimerDisabled = idleTimerDisabled;
    [self updateIdleTimer];
}

- (BOOL)isAppIdleTimerDisabled
{
    return _appIdleTimerDisabled;
}

- (void)setMediaModuleIdleTimerDisabled:(BOOL) idleTimerDisabled
{
    _mediaModuleIdleTimerDisabled = idleTimerDisabled;
    [self updateIdleTimer];
}

- (BOOL)isMediaModuleIdleTimerDisabled
{
    return _mediaModuleIdleTimerDisabled;
}

- (void)updateIdleTimer
{
    if (self.appIdleTimerDisabled || self.mediaModuleIdleTimerDisabled) {
        [UIApplication sharedApplication].idleTimerDisabled = YES;
    } else {
        [UIApplication sharedApplication].idleTimerDisabled = NO;
    }
}

@end
