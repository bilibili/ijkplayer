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

#define kIJKAudioQueueNumberBuffers (3)

@implementation IJKSDLAudioQueueController {
    AudioQueueRef _audioQueueRef;
    AudioQueueBufferRef _audioQueueBufferRefArray[kIJKAudioQueueNumberBuffers];
    BOOL _isPaused;

    volatile BOOL _isAborted;
}

- (id)initWithAudioSpec:(SDL_AudioSpec *)aSpec
{
    self = [super init];
    if (self) {
        if (aSpec == NULL) {
            self = nil;
            return nil;
        }
        _spec = *aSpec;

        /* Get the current format */
        _spec.format = AUDIO_S16SYS;
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
            AudioQueueEnqueueBuffer(audioQueueRef, _audioQueueBufferRefArray[i], 0, NULL);
        }

        status = AudioQueueStart(audioQueueRef, NULL);
        if (status != noErr) {
            NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
            self = nil;
            return nil;
        }
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

    _isPaused = NO;
    AudioSessionSetActive(true);
    OSStatus status = AudioQueueStart(_audioQueueRef, NULL);
    if (status != noErr)
        NSLog(@"AudioQueue: AudioQueueStart failed (%d)\n", (int)status);
}

- (void)pause
{
    if (!_audioQueueRef)
        return;

    _isPaused = YES;
    AudioSessionSetActive(false);
    OSStatus status = AudioQueuePause(_audioQueueRef);
    if (status != noErr)
        NSLog(@"AudioQueue: AudioQueuePause failed (%d)\n", (int)status);
}

- (void)flush
{
    if (!_audioQueueRef)
        return;

    AudioQueueFlush(_audioQueueRef);
}

- (void)stop
{
    AudioSessionSetActive(false);
    if (!_audioQueueRef)
        return;

    AudioQueueStop(_audioQueueRef, false);
    AudioQueueDispose(_audioQueueRef, false);
}

- (void)close
{
    [self stop];

    if (!_audioQueueRef)
        return;

    _audioQueueRef = NULL;
}

static void IJKSDLAudioQueueOuptutCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    @autoreleasepool {
        IJKSDLAudioQueueController* aqController = (__bridge IJKSDLAudioQueueController *) inUserData;

        if (!aqController || aqController->_isPaused) {
            memset(inBuffer->mAudioData, aqController.spec.silence, inBuffer->mAudioDataByteSize);
        } else {
            (*aqController.spec.callback)(aqController.spec.userdata, inBuffer->mAudioData, inBuffer->mAudioDataByteSize);
        }

        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}

@end
