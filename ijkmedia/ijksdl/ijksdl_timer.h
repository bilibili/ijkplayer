/*****************************************************************************
 * ijksdl_thread.h
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

#ifndef IJKSDL__IJKSDL_TIMER_H
#define IJKSDL__IJKSDL_TIMER_H

#include "ijksdl_stdinc.h"

void SDL_Delay(Uint32 ms);

Uint64 SDL_GetTickHR(void);


typedef struct SDL_Profiler {
    int64_t total_elapsed;
    int     total_counter;

    int64_t sample_elapsed;
    int     sample_counter;
    float   sample_per_seconds;
    int64_t average_elapsed;

    int64_t begin_time;

    int     max_sample;
} SDL_Profiler;

void    SDL_ProfilerReset(SDL_Profiler* profiler, int max_sample);
void    SDL_ProfilerBegin(SDL_Profiler* profiler);
int64_t SDL_ProfilerEnd(SDL_Profiler* profiler);

#endif
