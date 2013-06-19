/*
 * ff_ffmsg_queue.h
 *      based on PacketQueue in ffplay.c
 *
 * Copyright (c) 2000-2003 Fabrice Bellard
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKPLAYER__FF_FFMSG_QUEUE_H
#define IJKPLAYER__FF_FFMSG_QUEUE_H

#include "ff_ffinc.h"

typedef struct IjkMessage {
    int what;
    int arg1;
    int arg2;

    // optional
    void *data;
    void (*free_data)(void *data);

    int serial;
    struct IjkMessage *next;
} IjkMessage;

typedef struct IjkMessageQueue {
    IjkMessage *first_msg, *last_msg;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} IjkMessageQueue;

extern IjkMessage flush_msg;

static void ijkmsg_init_msg(IjkMessage *msg) {
    memset(msg, 0, sizeof(IjkMessage));
}

static IjkMessage *ijkmsg_obtain_msg() {
    IjkMessage *msg = (IjkMessage*) malloc(sizeof(IjkMessage));
    if (!msg)
        return NULL;

    ijkmsg_init_msg(msg);
    return msg;
}

static void ijkmsg_free_msg(IjkMessage **pmsg) {
    if (!pmsg || *pmsg)
        return;

    IjkMessage *msg = *pmsg;
    if (msg->free_data && msg->data) {
        msg->free_data(msg->data);
    }

    free(msg);
    *pmsg = NULL;
}

static int ijkmsg_queue_put_private(IjkMessageQueue *q, IjkMessage *msg)
{
    IjkMessage *msg1;

    if (q->abort_request)
        return -1;

    msg1 = malloc(sizeof(IjkMessage));
    if (!msg1)
        return -1;
    *msg1 = *msg;
    msg1->next = NULL;
    if (msg == &flush_msg)
        q->serial++;
    msg1->serial = q->serial;

    if (!q->last_msg)
        q->first_msg = msg1;
    else
        q->last_msg->next = msg1;
    q->last_msg = msg1;
    SDL_CondSignal(q->cond);
    return 0;
}

static int ijkmsg_queue_put(IjkMessageQueue *q, IjkMessage *msg)
{
    int ret;

    SDL_LockMutex(q->mutex);
    ret = ijkmsg_queue_put_private(q, msg);
    SDL_UnlockMutex(q->mutex);

    if (msg != &flush_msg && ret < 0)
        ijkmsg_free_msg(&msg);

    return ret;
}

static void ijkmsg_queue_init(IjkMessageQueue *q)
{
    memset(q, 0, sizeof(IjkMessageQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
    q->abort_request = 1;
}

static void ijkmsg_queue_flush(IjkMessageQueue *q)
{
    IjkMessage *msg, *msg1;

    SDL_LockMutex(q->mutex);
    for (msg = q->first_msg; msg != NULL; msg = msg1) {
        msg1 = msg->next;
        ijkmsg_free_msg(&msg);
    }
    q->last_msg = NULL;
    q->first_msg = NULL;
    SDL_UnlockMutex(q->mutex);
}

static void ijkmsg_queue_destroy(IjkMessageQueue *q)
{
    ijkmsg_queue_flush(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

static void ijkmsg_queue_abort(IjkMessageQueue *q)
{
    SDL_LockMutex(q->mutex);

    q->abort_request = 1;

    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
}

static void ijkmsg_queue_start(IjkMessageQueue *q)
{
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    ijkmsg_queue_put_private(q, &flush_msg);
    SDL_UnlockMutex(q->mutex);
}

/* return < 0 if aborted, 0 if no msg and > 0 if msg.  */
static int ijkmsg_queue_get(IjkMessageQueue *q, IjkMessage *msg, int block, int *serial)
{
    IjkMessage *msg1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        msg1 = q->first_msg;
        if (msg1) {
            q->first_msg = msg1->next;
            if (!q->first_msg)
                q->last_msg = NULL;
            *msg = *msg1;
            if (serial)
                *serial = msg1->serial;
            ijkmsg_free_msg(&msg1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

#endif
