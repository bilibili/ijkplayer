//
//  IJKFFPlayerDef.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 13-9-25.
//  Copyright (c) 2013年 bilibili. All rights reserved.
//

#import "IJKFFPlayerDef.h"

@implementation IJKFFPlayerMessage
@end

@implementation IJKFFPlayerMessagePool{
    NSMutableArray *_array;
}

- (IJKFFPlayerMessagePool *)init
{
    self = [super init];
    if (self) {
        _array = [[NSMutableArray alloc] init];
    }
    return self;
}

- (IJKFFPlayerMessage *) obtain
{
    IJKFFPlayerMessage *msg = nil;

    @synchronized(self) {
        NSUInteger count = [_array count];
        if (count > 0) {
            msg = [_array objectAtIndex:count - 1];
            [_array removeLastObject];
        }
    }

    if (!msg)
        msg = [[IJKFFPlayerMessage alloc] init];

    return msg;
}

- (void) recycle:(IJKFFPlayerMessage *)msg
{
    if (!msg)
        return;

    @synchronized(self) {
        if ([_array count] <= 10)
            [_array addObject:msg];
    }
}

@end