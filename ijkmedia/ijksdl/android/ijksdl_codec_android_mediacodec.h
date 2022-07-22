/*****************************************************************************
 * ijksdl_codec_android_mediacodec.h
 *****************************************************************************
 *
 * Copyright (c) 2014 Bilibili
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_H

#include "../ijksdl_stdinc.h"
#include <stdbool.h>
#include <jni.h>
#include <sys/types.h>
#include "../ijksdl_class.h"
#include "../ijksdl_mutex.h"
#include "ijksdl_codec_android_mediadef.h"
#include "android_nativewindow.h"

typedef struct SDL_AMediaCodecBufferInfo {
    int32_t offset;
    int32_t size;
    int64_t presentationTimeUs;
    uint32_t flags;
} SDL_AMediaCodecBufferInfo;

typedef struct SDL_AMediaFormat             SDL_AMediaFormat;
typedef struct SDL_AMediaCrypto             SDL_AMediaCrypto;

typedef struct SDL_AMediaCodec_Common       SDL_AMediaCodec_Common;
typedef struct SDL_AMediaCodec_Opaque       SDL_AMediaCodec_Opaque;
typedef struct SDL_AMediaCodec              SDL_AMediaCodec;
typedef struct SDL_AMediaCodec
{
    SDL_mutex    *mutex;
    volatile int  ref_count;

    SDL_Class              *opaque_class;
    SDL_AMediaCodec_Common *common;
    SDL_AMediaCodec_Opaque *opaque;
    bool                    is_configured;
    bool                    is_started;
    int                     object_serial;

    sdl_amedia_status_t (*func_delete)(SDL_AMediaCodec *acodec);

    sdl_amedia_status_t (*func_configure)(
        SDL_AMediaCodec* acodec,
        const SDL_AMediaFormat* aformat,
        ANativeWindow* surface,
        SDL_AMediaCrypto *crypto,
        uint32_t flags);
    sdl_amedia_status_t (*func_configure_surface)(
        JNIEnv*env,
        SDL_AMediaCodec* acodec,
        const SDL_AMediaFormat* aformat,
        jobject android_surface,
        SDL_AMediaCrypto *crypto,
        uint32_t flags);

    sdl_amedia_status_t     (*func_start)(SDL_AMediaCodec* acodec);
    sdl_amedia_status_t     (*func_stop)(SDL_AMediaCodec* acodec);
    sdl_amedia_status_t     (*func_flush)(SDL_AMediaCodec* acodec);

    ssize_t                 (*func_writeInputData)(SDL_AMediaCodec* acodec, size_t idx, const uint8_t *data, size_t size);

    ssize_t                 (*func_dequeueInputBuffer)(SDL_AMediaCodec* acodec, int64_t timeoutUs);
    sdl_amedia_status_t     (*func_queueInputBuffer)(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags);

    ssize_t                 (*func_dequeueOutputBuffer)(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs);
    SDL_AMediaFormat*       (*func_getOutputFormat)(SDL_AMediaCodec* acodec);
    sdl_amedia_status_t     (*func_releaseOutputBuffer)(SDL_AMediaCodec* acodec, size_t idx, bool render);

    bool                    (*func_isInputBuffersValid)(SDL_AMediaCodec* acodec);
} SDL_AMediaCodec;

int                     SDL_AMediaCodec_create_object_serial();

sdl_amedia_status_t     SDL_AMediaCodec_configure(
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    ANativeWindow* surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags);

sdl_amedia_status_t     SDL_AMediaCodec_configure_surface(
    JNIEnv*env,
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    jobject android_surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags);

void                    SDL_AMediaCodec_increaseReference(SDL_AMediaCodec *acodec);
void                    SDL_AMediaCodec_decreaseReference(SDL_AMediaCodec *acodec);
void                    SDL_AMediaCodec_decreaseReferenceP(SDL_AMediaCodec **acodec);

bool                    SDL_AMediaCodec_isConfigured(SDL_AMediaCodec *acodec);
bool                    SDL_AMediaCodec_isStarted(SDL_AMediaCodec *acodec);

sdl_amedia_status_t     SDL_AMediaCodec_start(SDL_AMediaCodec* acodec);
sdl_amedia_status_t     SDL_AMediaCodec_stop(SDL_AMediaCodec* acodec);
sdl_amedia_status_t     SDL_AMediaCodec_flush(SDL_AMediaCodec* acodec);

ssize_t                 SDL_AMediaCodec_writeInputData(SDL_AMediaCodec* acodec, size_t idx, const uint8_t *data, size_t size);

ssize_t                 SDL_AMediaCodec_dequeueInputBuffer(SDL_AMediaCodec* acodec, int64_t timeoutUs);
sdl_amedia_status_t     SDL_AMediaCodec_queueInputBuffer(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags);

ssize_t                 SDL_AMediaCodec_dequeueOutputBuffer(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs);
SDL_AMediaFormat*       SDL_AMediaCodec_getOutputFormat(SDL_AMediaCodec* acodec);
sdl_amedia_status_t     SDL_AMediaCodec_releaseOutputBuffer(SDL_AMediaCodec* acodec, size_t idx, bool render);

bool                    SDL_AMediaCodec_isInputBuffersValid(SDL_AMediaCodec* acodec);

int                     SDL_AMediaCodec_getSerial(SDL_AMediaCodec* acodec);
bool                    SDL_AMediaCodec_isSameSerial(SDL_AMediaCodec* acodec, int acodec_serial);

// extended
void                    SDL_AMediaCodecFake_abort(SDL_AMediaCodec* acodec);
void                    SDL_AMediaCodecFake_flushFakeFrames(SDL_AMediaCodec* acodec);
sdl_amedia_status_t     SDL_AMediaCodecFake_queueFakeFrame(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags);
ssize_t                 SDL_AMediaCodecFake_dequeueOutputBuffer(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs);
ssize_t                 SDL_AMediaCodecFake_dequeueFakeFrameOnly(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs);

#endif
