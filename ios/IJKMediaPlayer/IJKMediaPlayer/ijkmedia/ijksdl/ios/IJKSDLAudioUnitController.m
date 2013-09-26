/*
 * IJKSDLAudioUnitController.m
 *
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#import "IJKSDLAudioUnitController.h"
#include "ijkutil/ijkutil.h"

@implementation IJKSDLAudioUnitController {
    AudioUnit _auUnit;
}

- (id)initWithAudioSpec:(SDL_AudioSpec *)aSpec
{
    self = [super init];
    if (self) {
        if (aSpec == NULL) {
            self = nil;
            return nil;
        }
        self->_spec = *aSpec;

        AudioComponentDescription desc;
        desc.componentType = kAudioUnitType_Output;
        desc.componentSubType = kAudioUnitSubType_RemoteIO;
        desc.componentManufacturer = kAudioUnitManufacturer_Apple;
        desc.componentFlags = 0;
        desc.componentFlagsMask = 0;

        AudioComponent auComponent = AudioComponentFindNext(NULL, &desc);
        if (auComponent == NULL) {
            ALOGE("AudioUnit: AudioComponentFindNext failed");
            self = nil;
            return nil;
        }

        AudioUnit auUnit;
        OSStatus status = AudioComponentInstanceNew(auComponent, &auUnit);
        if (status != noErr) {
            ALOGE("AudioUnit: AudioComponentInstanceNew failed");
            self = nil;
            return nil;
        }

        UInt32 flag = 1;
        status = AudioUnitSetProperty(auUnit,
                                      kAudioOutputUnitProperty_EnableIO,
                                      kAudioUnitScope_Output,
                                      0,
                                      &flag,
                                      sizeof(flag));
        if (status != noErr) {
            ALOGE("AudioUnit: failed to set IO mode (%li)", status);
        }

        /* Get the current format */
        AudioStreamBasicDescription streamDescription;
        streamDescription.mSampleRate = self->_spec.freq;
        self->_spec.format = AUDIO_S16SYS;
        streamDescription.mFormatID = kAudioFormatLinearPCM;
        streamDescription.mFormatFlags = kLinearPCMFormatFlagIsPacked;
        streamDescription.mChannelsPerFrame = self->_spec.channels;
        streamDescription.mFramesPerPacket = 1;

        streamDescription.mBitsPerChannel = SDL_AUDIO_BITSIZE(self->_spec.format);
        if (SDL_AUDIO_ISBIGENDIAN(self->_spec.format))
            streamDescription.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
        if (SDL_AUDIO_ISFLOAT(self->_spec.format))
            streamDescription.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
        if (SDL_AUDIO_ISSIGNED(self->_spec.format))
            streamDescription.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;

        streamDescription.mBytesPerFrame = streamDescription.mBitsPerChannel * streamDescription.mChannelsPerFrame / 8;
        streamDescription.mBytesPerPacket = streamDescription.mBytesPerFrame * streamDescription.mFramesPerPacket;

        /* Set the desired format */
        UInt32 i_param_size = sizeof(streamDescription);
        status = AudioUnitSetProperty(auUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0,
                                      &streamDescription,
                                      i_param_size);
        if (status != noErr) {
            ALOGE("AudioUnit: failed to set stream format (%li)", status);
            self = nil;
            return nil;
        }

        /* Retrieve actual format */
        status = AudioUnitGetProperty(auUnit,
                                      kAudioUnitProperty_StreamFormat,
                                      kAudioUnitScope_Input,
                                      0,
                                      &streamDescription,
                                      &i_param_size);
        if (status != noErr) {
            ALOGE("AudioUnit: failed to verify stream format (%li)", status);
        }

        AURenderCallbackStruct callback;
        callback.inputProc = (AURenderCallback) RenderCallback;
        callback.inputProcRefCon = (__bridge void*) self;
        status = AudioUnitSetProperty(auUnit,
                                      kAudioUnitProperty_SetRenderCallback,
                                      kAudioUnitScope_Input,
                                      0, &callback, sizeof(callback));
        if (status != noErr) {
            ALOGE("AudioUnit: render callback setup failed (%li)", status);
            self = nil;
            return nil;
        }

        SDL_CalculateAudioSpec(&self->_spec);

        /* AU initiliaze */
        status = AudioUnitInitialize(auUnit);
        if (status != noErr) {
            ALOGE("AudioUnit: AudioUnitInitialize failed (%li)", status);
            self = nil;
            return nil;
        }

        /* start audio session so playback continues if mute switch is on */
        AudioSessionInitialize (NULL,
                                kCFRunLoopCommonModes,
                                NULL,
                                NULL);

        /* Set audio session to mediaplayback */
        UInt32 sessionCategory = kAudioSessionCategory_MediaPlayback;
        AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(sessionCategory), &sessionCategory);
        AudioSessionSetActive(true);

        self->_auUnit = auUnit;
    }
    return self;
}

- (void)dealloc
{
    [self close];
}

- (void)play
{
    if (!self->_auUnit)
        return;

    AudioOutputUnitStart(self->_auUnit);
    AudioSessionSetActive(true);
}

- (void)pause
{
    if (!self->_auUnit)
        return;

    AudioOutputUnitStop(self->_auUnit);
    AudioSessionSetActive(false);
}

- (void)flush
{
    if (!self->_auUnit)
        return;

    AudioOutputUnitStop(self->_auUnit);
}

- (void)stop
{
    AudioSessionSetActive(false);

    if (!self->_auUnit)
        return;

    OSStatus status = AudioOutputUnitStop(_auUnit);
    if (status != noErr)
        ALOGE("AudioUnit: failed to stop AudioUnit (%li)", status);

    status = AudioComponentInstanceDispose(_auUnit);
    if (status != noErr)
        ALOGE("AudioUnit: failed to dispose Audio Component instance (%li)", status);
}

- (void)close
{
    [self stop];

    if (!_auUnit)
        return;

    AURenderCallbackStruct callback;
    memset(&callback, 0, sizeof(AURenderCallbackStruct));
    AudioUnitSetProperty(_auUnit,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input, 0, &callback,
                         sizeof(callback));

    AudioComponentInstanceDispose(_auUnit);
    _auUnit = NULL;
}

static OSStatus RenderCallback(void                        *inRefCon,
                               AudioUnitRenderActionFlags  *ioActionFlags,
                               const AudioTimeStamp        *inTimeStamp,
                               UInt32                      inBusNumber,
                               UInt32                      inNumberFrames,
                               AudioBufferList             *ioData)
{
    @autoreleasepool {
        IJKSDLAudioUnitController* auController = (__bridge IJKSDLAudioUnitController *) inRefCon;

        if (!auController || auController.paused) {
            for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
                AudioBuffer *ioBuffer = &ioData->mBuffers[i];
                memset(ioBuffer->mData, auController.spec.silence, ioBuffer->mDataByteSize);
            }
            return noErr;
        }

        for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
            AudioBuffer *ioBuffer = &ioData->mBuffers[i];
            (*auController.spec.callback)(auController.spec.userdata, ioBuffer->mData, ioBuffer->mDataByteSize);
        }
        
        return noErr;
    }
}

@end
