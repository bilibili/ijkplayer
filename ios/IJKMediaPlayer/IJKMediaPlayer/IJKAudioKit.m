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

    OSStatus status = noErr;
    if (!_audioSessionInitialized) {
        status = AudioSessionInitialize(NULL,
                                        kCFRunLoopCommonModes,
                                        IjkAudioSessionInterruptionListener,
                                        NULL);
        if (status != noErr) {
            NSLog(@"IJKAudioKit: AudioSessionInitialize failed (%d)", (int)status);
            return;
        }
        _audioSessionInitialized = YES;
    }

    /* Set audio session to mediaplayback */
    UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
    status = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
    if (status != noErr) {
        NSLog(@"IJKAudioKit: AudioSessionSetProperty(kAudioSessionProperty_AudioCategory) failed (%d)", (int)status);
        return;
    }

    status = AudioSessionSetActive(true);
    if (status != noErr) {
        NSLog(@"IJKAudioKit: AudioSessionSetActive(true) failed (%d)", (int)status);
        return;
    }

    _delegate = delegate;
    return ;
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
                AudioSessionSetActive(false);
            });
            break;
        }
        case kAudioSessionEndInterruption: {
            NSLog(@"kAudioSessionEndInterruption\n");
            dispatch_async(dispatch_get_main_queue(), ^{
                AudioSessionSetActive(true);
                [delegate ijkAudioEndInterruption];
            });
            break;
        }
    }
}

@end
