/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_codec_android_mediacodec_internal.h"
#include "ijksdl/ijksdl_log.h"
#include "ijksdl/ijksdl_mutex.h"
#include "ijksdl/ijksdl_timer.h"
#include "ijksdl_codec_android_mediadef.h"

#define FAK_TRACE(...)
//#define FAK_TRACE ALOGE

sdl_amedia_status_t SDL_AMediaCodec_FakeFifo_init(SDL_AMediaCodec_FakeFifo *fifo)
{
    memset(fifo, 0, sizeof(SDL_AMediaCodec_FakeFifo));

    fifo->mutex = SDL_CreateMutex();
    fifo->wakeup_enqueue_cond = SDL_CreateCond();
    fifo->wakeup_dequeue_cond = SDL_CreateCond();

    return SDL_AMEDIA_OK;
}

void SDL_AMediaCodec_FakeFifo_abort(SDL_AMediaCodec_FakeFifo *fifo)
{
    SDL_LockMutex(fifo->mutex);
    fifo->should_abort = 1;
    SDL_CondSignal(fifo->wakeup_enqueue_cond);
    SDL_CondSignal(fifo->wakeup_dequeue_cond);
    SDL_UnlockMutex(fifo->mutex);
}

void SDL_AMediaCodec_FakeFifo_destroy(SDL_AMediaCodec_FakeFifo *fifo)
{
    if (!fifo)
        return;

    if (fifo->mutex)
        SDL_AMediaCodec_FakeFifo_abort(fifo);

    SDL_DestroyMutexP(&fifo->mutex);
    SDL_DestroyCondP(&fifo->wakeup_enqueue_cond);
    SDL_DestroyCondP(&fifo->wakeup_dequeue_cond);

    memset(fifo, 0, sizeof(SDL_AMediaCodec_FakeFifo));
}

sdl_amedia_status_t SDL_AMediaCodec_FakeFifo_queue(SDL_AMediaCodec_FakeFifo *fifo, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags)
{
    if (fifo->should_abort)
        return SDL_AMEDIA_ERROR_UNKNOWN;

    SDL_LockMutex(fifo->mutex);
    while (!fifo->should_abort) {
        if (fifo->size < FAKE_BUFFER_QUEUE_SIZE) {
            SDL_AMediaCodec_FakeFrame *fake = &fifo->fakes[fifo->end];
            fake->info.offset = offset;
            fake->info.size   = size;
            fake->info.presentationTimeUs = time;
            fake->info.flags  = flags;
            fake->index       = fifo->end;

            FAK_TRACE("%s, %d, %lld", __func__, fifo->end, time);

            fifo->end = (fifo->end + 1) % FAKE_BUFFER_QUEUE_SIZE;
            fifo->size++;
            SDL_CondSignal(fifo->wakeup_dequeue_cond);
            break;
        }

        SDL_CondWaitTimeout(fifo->wakeup_enqueue_cond, fifo->mutex, 1000);
    }
    SDL_UnlockMutex(fifo->mutex);

    if (fifo->should_abort)
        return SDL_AMEDIA_ERROR_UNKNOWN;

    return SDL_AMEDIA_OK;
}

ssize_t SDL_AMediaCodec_FakeFifo_dequeue(SDL_AMediaCodec_FakeFifo *fifo, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs)
{
    if (fifo->should_abort)
        return -1;

    int64_t  timeoutMs  = (timeoutUs + 999) / 1000;    
    ssize_t  dequeue_ret = -1;
    uint64_t wait_start = SDL_GetTickHR();
    int64_t  to_wait    = timeoutMs;

    SDL_LockMutex(fifo->mutex);
    while (!fifo->should_abort) {
        if (fifo->size > 0) {
            SDL_AMediaCodec_FakeFrame *fake = &fifo->fakes[fifo->begin];
            *info        = fake->info;
            info->flags |= AMEDIACODEC__BUFFER_FLAG_FAKE_FRAME;
            dequeue_ret  = fake->index;

            FAK_TRACE("%s, [%d]%lld", __func__, fifo->begin, info->presentationTimeUs);

            fifo->begin = (fifo->begin + 1) % FAKE_BUFFER_QUEUE_SIZE;
            fifo->size--;
            SDL_CondSignal(fifo->wakeup_enqueue_cond);
            break;
        }

        SDL_CondWaitTimeout(fifo->wakeup_dequeue_cond, fifo->mutex, to_wait);
        if (to_wait >= 0) {
            uint64_t now = SDL_GetTickHR();
            if (now < wait_start) {
                // tick overflow
                dequeue_ret = -1;
                break;
            } else {
                uint64_t elapsed = now - wait_start;
                if (elapsed >= timeoutMs) {
                    // timeout
                    dequeue_ret = -1;
                    break;
                } else {
                    to_wait = timeoutMs - elapsed;
                }
            }
        }
    }
    SDL_UnlockMutex(fifo->mutex);

    if (fifo->should_abort)
        return -1;

    return dequeue_ret;
}

void SDL_AMediaCodec_FakeFifo_flush(SDL_AMediaCodec_FakeFifo *fifo)
{
    if (fifo->should_abort)
        return;

    SDL_LockMutex(fifo->mutex);
    fifo->begin = 0;
    fifo->end   = 0;
    fifo->size  = 0;
    SDL_UnlockMutex(fifo->mutex);
}

int SDL_AMediaCodec_FakeFifo_size(SDL_AMediaCodec_FakeFifo *fifo)
{
    return fifo->size;
}
