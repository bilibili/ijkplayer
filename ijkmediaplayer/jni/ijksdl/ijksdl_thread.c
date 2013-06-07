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

#include <errno.h>
#include <assert.h>
#include "ijksdl_thread.h"

static void *SDL_RunThread(void *data)
{
    SDL_Thread *thread = data;
    thread->retval = thread->func(thread->data);
    return NULL;
}

SDL_Thread *SDL_CreateThreadEx(SDL_Thread *thread, int (*fn)(void *), void *data)
{
    thread->func = fn;
    thread->data = data;
    thread->retval = 0;
    if (pthread_create(&thread->id, NULL, SDL_RunThread, thread) != 0) {
        return NULL;
    }

    return thread;
}

void SDL_WaitThread(SDL_Thread *thread, int *status)
{
    assert(thread);
    if (!thread)
        return;

    pthread_join(thread->id, NULL);

    if (status)
        *status = thread->retval;
}
