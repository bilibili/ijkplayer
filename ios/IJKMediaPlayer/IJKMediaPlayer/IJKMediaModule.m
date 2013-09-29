/*
 * IJKMediaModule.m
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
