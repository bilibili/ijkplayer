//
//  IJKMediaModule.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-25.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKMediaModule.h"

@interface IJKMediaModule() {
    BOOL _isAppIsLockedOnResignActive;
}
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
    @synchronized(self) {
        [self.appLock unlock];
    }
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
    _isAppIsLockedOnResignActive = TRUE;
    NSLog(@"IJKMediaModule:applicationWillResignActive lockApp");
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if (_isAppIsLockedOnResignActive) {
        [self unlockApp];
        _isAppIsLockedOnResignActive = FALSE;
        NSLog(@"IJKMediaModule:applicationDidEnterBackground unlockApp");
    }
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if (_isAppIsLockedOnResignActive) {
        [self unlockApp];
        _isAppIsLockedOnResignActive = FALSE;
        NSLog(@"IJKMediaModule:applicationWillEnterForeground unlockApp");
    }
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    if (_isAppIsLockedOnResignActive) {
        [self unlockApp];
        _isAppIsLockedOnResignActive = FALSE;
        NSLog(@"IJKMediaModule:applicationWillTerminate unlockApp");
    }
}

@end
