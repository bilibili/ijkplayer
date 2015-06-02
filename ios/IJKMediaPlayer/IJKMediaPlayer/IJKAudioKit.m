/*
 * IJKAudioKit.m
 *
 * Copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
 *
 * based on https://github.com/kolyvan/kxmovie
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

#import "IJKAudioKit.h"

#import <AVFoundation/AVFoundation.h>

@implementation IJKAudioKit {
    __weak id<IJKAudioSessionDelegate> _delegate;

    BOOL _audioSessionInitialized;
}

+ (IJKAudioKit *)sharedInstance
{
    static IJKAudioKit *sAudioKit = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sAudioKit = [[IJKAudioKit alloc] init];
    });
    return sAudioKit;
}

- (void)setupAudioSession:(id<IJKAudioSessionDelegate>) delegate
{
    _delegate = nil;
    if (delegate == nil) {
        return;
    }

    if (!_audioSessionInitialized) {
        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector(handleInterruption:)
                                                     name: AVAudioSessionInterruptionNotification
                                                   object: [AVAudioSession sharedInstance]];
        _audioSessionInitialized = YES;
    }

    /* Set audio session to mediaplayback */
    NSError *error = nil;
    if (NO == [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:&error]) {
        NSLog(@"IJKAudioKit: AVAudioSession.setCategory() failed: %@\n", error ? [error localizedDescription] : @"nil");
        return;
    }

    error = nil;
    if (NO == [[AVAudioSession sharedInstance] setActive:YES error:&error]) {
        NSLog(@"IJKAudioKit: AVAudioSession.setActive(YES) failed: %@\n", error ? [error localizedDescription] : @"nil");
        return;
    }

    _delegate = delegate;
    return ;
}

- (void)handleInterruption:(NSNotification *)notification
{
    int reason = [[[notification userInfo] valueForKey:AVAudioSessionInterruptionTypeKey] intValue];
    switch (reason) {
        case AVAudioSessionInterruptionTypeBegan: {
            NSLog(@"AVAudioSessionInterruptionTypeBegan\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                [_delegate ijkAudioBeginInterruption];
                [[AVAudioSession sharedInstance] setActive:YES error:nil];
            });
            break;
        }
        case AVAudioSessionInterruptionTypeEnded: {
            NSLog(@"AVAudioSessionInterruptionTypeEnded\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                [_delegate ijkAudioEndInterruption];
                @try {
                    [[AVAudioSession sharedInstance] setActive:NO error:nil];
                } @catch (NSException *exception) {
                    NSLog(@"failed to inactive AVAudioSession\n");
                }
            });
            break;
        }
    }
}

static void IjkAudioSessionInterruptionListener(void *inClientData, UInt32 inInterruptionState)
{
    id<IJKAudioSessionDelegate> delegate = [IJKAudioKit sharedInstance]->_delegate;
    if (delegate == nil)
        return;

    switch (inInterruptionState) {
        case kAudioSessionBeginInterruption: {
            NSLog(@"kAudioSessionBeginInterruption\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                [delegate ijkAudioBeginInterruption];
                [[AVAudioSession sharedInstance] setActive:YES error:nil];
            });
            break;
        }
        case kAudioSessionEndInterruption: {
            NSLog(@"kAudioSessionEndInterruption\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                [delegate ijkAudioEndInterruption];
                @try {
                    [[AVAudioSession sharedInstance] setActive:NO error:nil];
                } @catch (NSException *exception) {
                    NSLog(@"failed to inactive AVAudioSession\n");
                }
            });
            break;
        }
    }
}

@end
