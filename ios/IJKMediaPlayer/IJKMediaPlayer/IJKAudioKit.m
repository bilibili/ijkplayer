//
//  IJKAudioKit.m
//  IJKMediaPlayer
//
//  Created by ZhangRui on 14-1-4.
//  Copyright (c) 2014年 bilibili. All rights reserved.
//

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
                AudioSessionSetActive(false);
                [delegate ijkAudioBeginInterruption];
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
