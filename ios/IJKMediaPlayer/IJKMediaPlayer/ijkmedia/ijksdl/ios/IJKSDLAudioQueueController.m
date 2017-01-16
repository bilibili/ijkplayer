/*
 * IJKSDLAudioQueueController.m
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

#import "IJKSDLAudioQueueController.h"
#import "IJKSDLAudioKit.h"
#import "ijksdl_log.h"

#import <AVFoundation/AVFoundation.h>

#define kIJKAudioQueueNumberBuffers (3)

@implementation IJKSDLAudioQueueController {
    AudioQueueRef _audioQueueRef;
    AudioQueueBufferRef _audioQueueBufferRefArray[kIJKAudioQueueNumberBuffers];
    BOOL _isPaused;
    BOOL _isStopped;

    volatile BOOL _isAborted;
    NSLock *_lock;
}

- (id)initWithAudioSpec:(const SDL_AudioSpec *)aSpec
{
    self = [super init];
    if (self) {
        if (aSpec == NULL) {
            self = nil;
            return nil;
        }
        _spec = *aSpec;

        if (aSpec->format != AUDIO_S16SYS) {
            NSLog(@"aout_open_audio: unsupported format %d\n", (int)aSpec->format);
            return nil;
        }

        if (aSpec->channels > 2) {
            NSLog(@"aout_open_audio: unsupported channels %d\n", (int)aSpec->channels);
            return nil;
        }

        /* Get the current format */
        AudioStreamBasicDescription streamDescription;
        IJKSDLGetAudioStreamBasicDescriptionFromSpec(&_spec, &streamDescription);

        /* Set the desired format */
        AudioQueueRef audioQueueRef;
        OSStatus status = AudioQueueNewOutput(&streamDescription,
                                              IJKSDLAudioQueueOuptutCallback,
                                              (__bridge void *) self,
                                              NULL,
                                              kCFRunLoopCommonModes,
                                              0,
                                              &audioQueueRef);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueNewOutput failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }

        UInt32 propValue = 1;
        AudioQueueSetProperty(audioQueueRef, kAudioQueueProperty_EnableTimePitch, &propValue, sizeof(propValue));
        propValue = 1;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        propValue = kAudioQueueTimePitchAlgorithm_Spectral;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchAlgorithm, &propValue, sizeof(propValue));

        status = AudioQueueStart(audioQueueRef, NULL);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }

        SDL_CalculateAudioSpec(&_spec);

        _audioQueueRef = audioQueueRef;

        for (int i = 0;i < kIJKAudioQueueNumberBuffers; i++)
        {
            AudioQueueAllocateBuffer(audioQueueRef, _spec.size, &_audioQueueBufferRefArray[i]);
            _audioQueueBufferRefArray[i]->mAudioDataByteSize = _spec.size;
            memset(_audioQueueBufferRefArray[i]->mAudioData, 0, _spec.size);
            AudioQueueEnqueueBuffer(audioQueueRef, _audioQueueBufferRefArray[i], 0, NULL);
        }
        /*-
        status = AudioQueueStart(audioQueueRef, NULL);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }
         */

        _isStopped = NO;

        _lock = [[NSLock alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [self close];
}

- (void)play
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        _isPaused = NO;
        NSError *error = nil;
        if (NO == [[AVAudioSession sharedInstance] setActive:YES error:&error]) {
            NSLog(@"AudioQueue: AVAudioSession.setActive(YES) failed: %@\n", error ? [error localizedDescription] : @"nil");
        }

        OSStatus status = AudioQueueStart(_audioQueueRef, NULL);
        if (status != noErr)
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
    }
}

- (void)pause
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        if (_isStopped)
            return;

        _isPaused = YES;
        OSStatus status = AudioQueuePause(_audioQueueRef);
        if (status != noErr)
            NSLog(@"AudioQueue: AudioQueuePause failed (%d)\n", (int)status);
    }
}

- (void)flush
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        if (_isStopped)
            return;

        AudioQueueFlush(_audioQueueRef);
    }
}

- (void)stop
{
    if (!_audioQueueRef)
        return;

    @synchronized(_lock) {
        if (_isStopped)
            return;

        _isStopped = YES;
    }

    // do not lock AudioQueueStop, or may be run into deadlock
    AudioQueueStop(_audioQueueRef, true);
    AudioQueueDispose(_audioQueueRef, true);
}

- (void)close
{
    [self stop];
    _audioQueueRef = nil;
}

- (void)setPlaybackRate:(float)playbackRate
{
    if (fabsf(playbackRate - 1.0f) <= 0.000001) {
        UInt32 propValue = 1;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, 1.0f);
    } else {
        UInt32 propValue = 0;
        AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, playbackRate);
    }
}

- (void)setPlaybackVolume:(float)playbackVolume
{
    float aq_volume = playbackVolume;
    if (fabsf(aq_volume - 1.0f) <= 0.000001) {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, 1.f);
    } else {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, aq_volume);
    }
}

- (double)get_latency_seconds
{
    return ((double)(kIJKAudioQueueNumberBuffers)) * _spec.samples / _spec.freq;
}

static void IJKSDLAudioQueueOuptutCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    @autoreleasepool {
        IJKSDLAudioQueueController* aqController = (__bridge IJKSDLAudioQueueController *) inUserData;

        if (!aqController) {
            // do nothing;
        } else if (aqController->_isPaused || aqController->_isStopped) {
            memset(inBuffer->mAudioData, aqController.spec.silence, inBuffer->mAudioDataByteSize);
        } else {
            (*aqController.spec.callback)(aqController.spec.userdata, inBuffer->mAudioData, inBuffer->mAudioDataByteSize);
        }

        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}

@end
