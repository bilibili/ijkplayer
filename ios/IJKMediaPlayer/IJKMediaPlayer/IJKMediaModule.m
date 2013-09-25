//
//  IJKMediaModule.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-25.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMediaModule.h"

@interface IJKMediaModule()
@property(atomic,strong) NSRecursiveLock *appLock;
@end;

@implementation IJKMediaModule

+ (IJKMediaModule *) sharedModule
{
    static IJKMediaModule *sharedSingleton;

    @synchronized(self)
    {
        if (sharedSingleton == NULL) {
            sharedSingleton = [[self alloc] init];
            sharedSingleton.appLock = [[NSRecursiveLock alloc] init];
        }

        return(sharedSingleton);
    }
}

- (void) lockApp
{
    [self.appLock lock];
}

- (void) unlockApp
{
    [self.appLock unlock];
}

- (BOOL) tryLockActiveApp
{
    if (![self.appLock tryLock])
        return NO;

    if ([UIApplication sharedApplication].applicationState != UIApplicationStateActive) {
        [self.appLock unlock];
        return NO;
    }

    return YES;
}

#pragma mark AppDelegate

- (void)applicationWillResignActive:(UIApplication *)application
{
    [self lockApp];
    NSLog(@"applicationWillResignActive: state: %d", [UIApplication sharedApplication].applicationState);
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    [self unlockApp];
    NSLog(@"applicationDidEnterBackground: state: %d", [UIApplication sharedApplication].applicationState);
}

@end
