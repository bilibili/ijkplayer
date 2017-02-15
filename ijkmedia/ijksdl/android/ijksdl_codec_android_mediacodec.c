/*****************************************************************************
 * ijksdl_codec_android_mediacodec.c
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

#include "ijksdl_codec_android_mediacodec.h"
#include <assert.h>
#include "ijksdl/ijksdl_log.h"
#include "ijksdl_codec_android_mediacodec_internal.h"

static volatile int g_amediacodec_object_serial;

typedef struct SDL_AMediaCodec_Common
{
    SDL_AMediaCodec_FakeFifo fake_fifo;
} SDL_AMediaCodec_Common;

int SDL_AMediaCodec_create_object_serial()
{
    int object_serial = __sync_add_and_fetch(&g_amediacodec_object_serial, 1);
    if (object_serial == 0)
        object_serial = __sync_add_and_fetch(&g_amediacodec_object_serial, 1);
    return object_serial;
}

// FIXME: release SDL_AMediaCodec
sdl_amedia_status_t SDL_AMediaCodec_delete(SDL_AMediaCodec* acodec)
{
    if (!acodec)
        return SDL_AMEDIA_OK;

    assert(acodec->func_delete);
    return acodec->func_delete(acodec);
}

sdl_amedia_status_t SDL_AMediaCodec_deleteP(SDL_AMediaCodec** acodec)
{
    if (!acodec)
        return SDL_AMEDIA_OK;
    sdl_amedia_status_t ret = SDL_AMediaCodec_delete(*acodec);
    *acodec = NULL;
    return ret;
}

sdl_amedia_status_t SDL_AMediaCodec_configure(
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    ANativeWindow* surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags)
{
    assert(acodec->func_configure);
    sdl_amedia_status_t ret = acodec->func_configure(acodec, aformat, surface, crypto, flags);
    acodec->is_configured = true;
    acodec->is_started    = false;
    return ret;
}

sdl_amedia_status_t SDL_AMediaCodec_configure_surface(
    JNIEnv*env,
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    jobject android_surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags)
{
    assert(acodec->func_configure_surface);
    sdl_amedia_status_t ret = acodec->func_configure_surface(env, acodec, aformat, android_surface, crypto, flags);
    acodec->is_configured = true;
    acodec->is_started    = false;
    return ret;
}

void SDL_AMediaCodec_increaseReference(SDL_AMediaCodec *acodec)
{
    assert(acodec);
    int ref_count = __sync_add_and_fetch(&acodec->ref_count, 1);
    ALOGD("%s(): ref=%d\n", __func__, ref_count);
}

void SDL_AMediaCodec_decreaseReference(SDL_AMediaCodec *acodec)
{
    if (!acodec)
        return;

    int ref_count = __sync_sub_and_fetch(&acodec->ref_count, 1);
    ALOGD("%s(): ref=%d\n", __func__, ref_count);
    if (ref_count == 0) {
        if (SDL_AMediaCodec_isStarted(acodec)) {
            SDL_AMediaCodec_stop(acodec);
        }
        SDL_AMediaCodec_delete(acodec);
    }
}

void SDL_AMediaCodec_decreaseReferenceP(SDL_AMediaCodec **acodec)
{
    if (!acodec)
        return;

    SDL_AMediaCodec_decreaseReference(*acodec);
    *acodec = NULL;
}

bool SDL_AMediaCodec_isConfigured(SDL_AMediaCodec *acodec)
{
    return acodec->is_configured;
}

bool SDL_AMediaCodec_isStarted(SDL_AMediaCodec *acodec)
{
    return acodec->is_started;
}

sdl_amedia_status_t SDL_AMediaCodec_start(SDL_AMediaCodec* acodec)
{
    assert(acodec->func_start);
    sdl_amedia_status_t amc_ret = acodec->func_start(acodec);
    if (amc_ret == SDL_AMEDIA_OK) {
        acodec->is_started = true;
    }
    return amc_ret;
}

sdl_amedia_status_t SDL_AMediaCodec_stop(SDL_AMediaCodec* acodec)
{
    assert(acodec->func_stop);
    acodec->is_started = false;
    SDL_AMediaCodec_FakeFifo_abort(&acodec->common->fake_fifo);
    return acodec->func_stop(acodec);
}

sdl_amedia_status_t SDL_AMediaCodec_flush(SDL_AMediaCodec* acodec)
{
    assert(acodec->func_flush);
    SDL_AMediaCodec_FakeFifo_flush(&acodec->common->fake_fifo);
    return acodec->func_flush(acodec);
}

ssize_t SDL_AMediaCodec_writeInputData(SDL_AMediaCodec* acodec, size_t idx, const uint8_t *data, size_t size)
{
    assert(acodec->func_writeInputData);
    return acodec->func_writeInputData(acodec, idx, data, size);
}

ssize_t SDL_AMediaCodec_dequeueInputBuffer(SDL_AMediaCodec* acodec, int64_t timeoutUs)
{
    assert(acodec->func_dequeueInputBuffer);
    return acodec->func_dequeueInputBuffer(acodec, timeoutUs);
}

sdl_amedia_status_t SDL_AMediaCodec_queueInputBuffer(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags)
{
    assert(acodec->func_queueInputBuffer);
    if (flags & AMEDIACODEC__BUFFER_FLAG_FAKE_FRAME) {
        return SDL_AMediaCodec_FakeFifo_queueInputBuffer(&acodec->common->fake_fifo, idx, offset, size, time, flags);
    }

    return acodec->func_queueInputBuffer(acodec, idx, offset, size, time, flags);
}

ssize_t SDL_AMediaCodec_dequeueOutputBuffer(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs)
{
    assert(acodec->func_dequeueOutputBuffer);
    return acodec->func_dequeueOutputBuffer(acodec, info, timeoutUs);
}

SDL_AMediaFormat* SDL_AMediaCodec_getOutputFormat(SDL_AMediaCodec* acodec)
{
    assert(acodec->func_getOutputFormat);
    return acodec->func_getOutputFormat(acodec);
}

sdl_amedia_status_t SDL_AMediaCodec_releaseOutputBuffer(SDL_AMediaCodec* acodec, size_t idx, bool render)
{
    assert(acodec->func_releaseOutputBuffer);
    return acodec->func_releaseOutputBuffer(acodec, idx, render);
}

bool SDL_AMediaCodec_isInputBuffersValid(SDL_AMediaCodec* acodec)
{
    assert(acodec->func_isInputBuffersValid);
    return acodec->func_isInputBuffersValid(acodec);
}

int SDL_AMediaCodec_getSerial(SDL_AMediaCodec* acodec)
{
    if (!acodec)
        return 0;
    return acodec->object_serial;
}

bool SDL_AMediaCodec_isSameSerial(SDL_AMediaCodec* acodec, int acodec_serial)
{
    if (acodec == NULL)
        return false;
    return acodec->object_serial == acodec_serial;
}

SDL_AMediaCodec *SDL_AMediaCodec_CreateInternal(size_t opaque_size)
{
    SDL_AMediaCodec *acodec = (SDL_AMediaCodec*) mallocz(sizeof(SDL_AMediaCodec));
    if (!acodec)
        return NULL;

    acodec->mutex = SDL_CreateMutex();
    if (acodec->mutex == NULL)
        goto fail;

    acodec->opaque = mallocz(opaque_size);
    if (!acodec->opaque)
        goto fail;

    acodec->common = mallocz(sizeof(SDL_AMediaCodec_Common));
    if (!acodec->common)
        goto fail;

    SDL_AMediaCodec_FakeFifo_init(&acodec->common->fake_fifo);

    return acodec;
fail:
    SDL_AMediaCodec_FreeInternal(acodec);
    return NULL;
}

void SDL_AMediaCodec_FreeInternal(SDL_AMediaCodec *acodec)
{
    if (!acodec)
        return;

    if (acodec->common) {
        SDL_AMediaCodec_FakeFifo_destroy(&acodec->common->fake_fifo);
        free(acodec->common);
    }

    free(acodec->opaque);

    if (acodec->mutex)
        SDL_DestroyMutexP(&acodec->mutex);

    memset(acodec, 0, sizeof(SDL_AMediaCodec));
    free(acodec);
}

void SDL_AMediaCodecFake_abort(SDL_AMediaCodec* acodec)
{
    SDL_AMediaCodec_FakeFifo_abort(&acodec->common->fake_fifo);
}

void SDL_AMediaCodecFake_flushFakeFrames(SDL_AMediaCodec* acodec)
{
    SDL_AMediaCodec_FakeFifo_flush(&acodec->common->fake_fifo);
}

sdl_amedia_status_t SDL_AMediaCodecFake_queueFakeFrame(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags)
{
    return SDL_AMediaCodec_FakeFifo_queueInputBuffer(&acodec->common->fake_fifo, idx, offset, size, time, flags);
}

ssize_t SDL_AMediaCodecFake_dequeueOutputBuffer(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs)
{    
    if (SDL_AMediaCodec_FakeFifo_size(&acodec->common->fake_fifo) > 0) {
        ssize_t ret = SDL_AMediaCodec_FakeFifo_dequeueOutputBuffer(&acodec->common->fake_fifo, info, 0);
        if (ret >= 0)
            return ret;
    }

    assert(acodec->func_dequeueOutputBuffer);
    return acodec->func_dequeueOutputBuffer(acodec, info, timeoutUs);
}

ssize_t SDL_AMediaCodecFake_dequeueFakeFrameOnly(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs)
{
    return SDL_AMediaCodec_FakeFifo_dequeueOutputBuffer(&acodec->common->fake_fifo, info, 0);
}
