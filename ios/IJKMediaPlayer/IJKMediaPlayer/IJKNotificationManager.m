/*
 * IJKNotificationManager.m
 *
 * Copyright (c) 2016 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKNotificationManager.h"

@implementation IJKNotificationManager
{
    NSMutableDictionary *_registeredNotifications;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _registeredNotifications = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)addObserver:(nonnull id)observer
           selector:(nonnull SEL)aSelector
               name:(nullable NSString *)aName
             object:(nullable id)anObject
{
    [[NSNotificationCenter defaultCenter] addObserver:observer
                                             selector:aSelector
                                                 name:aName
                                               object:anObject];

    [_registeredNotifications setValue:aName forKey:aName];
}

- (void)removeAllObservers:(nonnull id)observer
{
    for (NSString *name in [_registeredNotifications allKeys]) {
        [[NSNotificationCenter defaultCenter] removeObserver:observer
                                                        name:name
                                                      object:nil];
    }
}

@end
