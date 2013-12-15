/*
 * IJKFFMoviePlayerDef.m
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

#import "IJKFFMoviePlayerDef.h"

@implementation IJKFFMoviePlayerMessage
@end

@implementation IJKFFMoviePlayerMessagePool{
    NSMutableArray *_array;
}

- (IJKFFMoviePlayerMessagePool *)init
{
    self = [super init];
    if (self) {
        _array = [[NSMutableArray alloc] init];
    }
    return self;
}

- (IJKFFMoviePlayerMessage *) obtain
{
    IJKFFMoviePlayerMessage *msg = nil;

    @synchronized(self) {
        NSUInteger count = [_array count];
        if (count > 0) {
            msg = [_array objectAtIndex:count - 1];
            [_array removeLastObject];
        }
    }

    if (!msg)
        msg = [[IJKFFMoviePlayerMessage alloc] init];

    return msg;
}

- (void) recycle:(IJKFFMoviePlayerMessage *)msg
{
    if (!msg)
        return;

    @synchronized(self) {
        if ([_array count] <= 10)
            [_array addObject:msg];
    }
}

@end
