/*****************************************************************************
 * ijksdl_thread.c
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_timer.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#if defined(__APPLE__)
#include <mach/mach_time.h>

static int g_is_mach_base_info_inited = 0;
static kern_return_t g_mach_base_info_ret = 0;
static mach_timebase_info_data_t g_mach_base_info;

/* nanosleep is not included in c99, just a workaround for CocoaPods */
int nanosleep(const struct timespec *, struct timespec *) __DARWIN_ALIAS_C(nanosleep);
#endif

void SDL_Delay(Uint32 ms)
{
    int was_error;
    struct timespec elapsed, tv;

    /* Set the timeout interval */
    elapsed.tv_sec = ms / 1000;
    elapsed.tv_nsec = (ms % 1000) * 1000000;
    do {
        tv.tv_sec = elapsed.tv_sec;
        tv.tv_nsec = elapsed.tv_nsec;
        was_error = nanosleep(&tv, &elapsed);
    } while (was_error);
}

Uint64 SDL_GetTickHR(void)
{
    Uint64 clock;
#if defined(__ANDROID__)
    struct timespec now;
#ifdef CLOCK_MONOTONIC_COARSE
    clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
#else
    clock_gettime(CLOCK_MONOTONIC_HR, &now);
#endif
    clock = now.tv_sec * 1000 + now.tv_nsec / 1000000;
#elif defined(__APPLE__)
    if (!g_is_mach_base_info_inited) {
        g_mach_base_info_ret = mach_timebase_info(&g_mach_base_info);
        g_is_mach_base_info_inited = 1;
    }
    if (g_mach_base_info_ret == 0) {
        uint64_t now = mach_absolute_time();
        clock = now * g_mach_base_info.numer / g_mach_base_info.denom / 1000000;
    } else {
        struct timeval now;
        gettimeofday(&now, NULL);
        clock = now.tv_sec  * 1000 + now.tv_usec / 1000;
    }
#endif
    return (clock);
}

void SDL_ProfilerReset(SDL_Profiler* profiler, int max_sample)
{
    memset(profiler, 0, sizeof(SDL_Profiler));
    if (max_sample < 0)
        profiler->max_sample = 3;
    else
        profiler->max_sample = max_sample;
}

void SDL_ProfilerBegin(SDL_Profiler* profiler)
{
    profiler->begin_time = SDL_GetTickHR();
}

int64_t SDL_ProfilerEnd(SDL_Profiler* profiler)
{
    int64_t delta = SDL_GetTickHR() - profiler->begin_time;

    if (profiler->max_sample > 0) {
        profiler->total_elapsed += delta;
        profiler->total_counter += 1;

        profiler->sample_elapsed += delta;
        profiler->sample_counter  += 1;

        if (profiler->sample_counter > profiler->max_sample) {
            profiler->sample_elapsed -= profiler->average_elapsed;
            profiler->sample_counter -= 1;
        }

        if (profiler->sample_counter > 0) {
            profiler->average_elapsed = profiler->sample_elapsed / profiler->sample_counter;
        }
        if (profiler->sample_elapsed > 0) {
            profiler->sample_per_seconds = profiler->sample_counter * 1000.f / profiler->sample_elapsed;
        }
    }

    return delta;
}
