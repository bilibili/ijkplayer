/*****************************************************************************
 * ijksdl_codec_android_mediacodec_internal.h
 *****************************************************************************
 *
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_INTERNAL_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_INTERNAL_H

#include "ijksdl_codec_android_mediacodec.h"

SDL_AMediaCodec *SDL_AMediaCodec_CreateInternal(size_t opaque_size);
void             SDL_AMediaCodec_FreeInternal(SDL_AMediaCodec *acodec);

#define FAKE_BUFFER_QUEUE_SIZE 5

typedef struct SDL_AMediaCodec_FakeFrame {
    size_t index;
    SDL_AMediaCodecBufferInfo info;
} SDL_AMediaCodec_FakeFrame;

typedef struct SDL_AMediaCodec_FakeFifo {
    SDL_AMediaCodec_FakeFrame fakes[FAKE_BUFFER_QUEUE_SIZE];
    int begin;
    int end;
    int size;
    int should_abort;

    SDL_mutex *mutex;
    SDL_cond  *wakeup_enqueue_cond;
    SDL_cond  *wakeup_dequeue_cond;
} SDL_AMediaCodec_FakeFifo;

sdl_amedia_status_t SDL_AMediaCodec_FakeFifo_init(SDL_AMediaCodec_FakeFifo *fifo);
void                SDL_AMediaCodec_FakeFifo_abort(SDL_AMediaCodec_FakeFifo *fifo);
void                SDL_AMediaCodec_FakeFifo_destroy(SDL_AMediaCodec_FakeFifo *fifo);
ssize_t             SDL_AMediaCodec_FakeFifo_dequeueInputBuffer(SDL_AMediaCodec_FakeFifo* fifo, int64_t timeoutUs);
sdl_amedia_status_t SDL_AMediaCodec_FakeFifo_queueInputBuffer(SDL_AMediaCodec_FakeFifo *fifo, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags);
ssize_t             SDL_AMediaCodec_FakeFifo_dequeueOutputBuffer(SDL_AMediaCodec_FakeFifo *fifo, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs);
void                SDL_AMediaCodec_FakeFifo_flush(SDL_AMediaCodec_FakeFifo *fifo);
int                 SDL_AMediaCodec_FakeFifo_size(SDL_AMediaCodec_FakeFifo *fifo);

#endif

