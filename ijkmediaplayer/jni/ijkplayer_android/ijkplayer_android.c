/*
 * ijkplayer_android.c
 *
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

#include "ijkplayer_android.h"

#include <assert.h>
#include "ffplay/ff_fferror.h"
#include "ffplay/ff_ffplay.h"

#define MPST_CHECK_NOT_RET_INT(real, expected, errcode) \
    do { \
        if (real == expected) return errcode; \
    } while(0)

#define MPST_CHECK_NOT_RET(real, expected) \
    MPST_CHECK_NOT_RET_INT(real, expected, EIJK_INVALID_STATE)

typedef struct IjkMediaPlayer {
    volatile int ref_count;
    pthread_mutex_t mutex;
    FFPlayer *ffplayer;

    // FIXME: IjkMessageQueue msg_queue;

    int mp_state;
    char *data_source;
} IjkMediaPlayer;

inline static void destroy_mp(IjkMediaPlayer **pmp)
{
    if (!pmp)
        return;

    IjkMediaPlayer *mp = *pmp;
    if (!mp)
        return;

    FFPlayer *ffp = mp->ffplayer;
    if (ffp) {
        SDL_AoutFree(ffp->aout);
        SDL_VoutFree(ffp->vout);
        ijkff_destroy_ffplayer(&mp->ffplayer);
    }

    // FIXME: ijkmsg_queue_destroy(&mp->msg_queue);
    pthread_mutex_destroy(&mp->mutex);

    free(mp->data_source);

    memset(mp, 0, sizeof(IjkMediaPlayer));
    free(mp);
    *pmp = NULL;
}

void ijkmp_global_init()
{
    ijkff_global_init();
}

void ijkmp_global_uninit()
{
    ijkff_global_uninit();
}

static void ijkmp_msg_handler(void *opaque, int what, int arg1, int arg2, void* data)
{
    // IjkMediaPlayer *mp = (IjkMediaPlayer *) opaque;
    // FFPlayer *ffp = mp->ffplayer;

    // FIXME: implement
}

void ijkmp_setup_internal(IjkMediaPlayer *mp) {
    FFPlayer *ffp = mp->ffplayer;

    ffp->msg_opaque = mp;
    ffp->msg_handler = ijkmp_msg_handler;
    // FIXME: ijkmsg_queue_flush(&mp->msg_queue);
    // FIXME: ijkmsg_queue_start(&mp->msg_queue);
}

IjkMediaPlayer *ijkmp_create()
{
    IjkMediaPlayer *mp = (IjkMediaPlayer *) malloc(sizeof(IjkMediaPlayer));
    if (!mp) {
        return NULL;
    }
    memset(mp, 0, sizeof(IjkMediaPlayer));

    FFPlayer *ffp = ijkff_create_ffplayer();
    if (!mp) {
        free(mp);
        return NULL;
    }

    pthread_mutex_init(&mp->mutex, NULL);
    // FIXME: ijkmsg_queue_init(&mp->msg_queue);

    ijkmp_inc_ref(mp);

    mp->ffplayer = ffp;
    ijkmp_setup_internal(mp);

    return mp;
}

void ijkmp_shutdown_l(IjkMediaPlayer *mp)
{
    assert(mp);

    // FIXME: ijkmsg_queue_abort(&mp->msg_queue);

    if (mp->ffplayer) {
        ijkff_stop_l(mp->ffplayer);
        ijkff_wait_stop_l(mp->ffplayer);
    }
}

void ijkmp_shutdown(IjkMediaPlayer *mp)
{
    return ijkmp_shutdown_l(mp);
}

void ijkmp_reset_l(IjkMediaPlayer *mp)
{
    assert(mp);

    ijkmp_shutdown_l(mp);
    ijkff_reset(mp->ffplayer);

    free(mp->data_source);
    mp->data_source = NULL;
    mp->mp_state = MP_STATE_IDLE;

    ijkmp_setup_internal(mp);
}

void ijkmp_reset(IjkMediaPlayer *mp)
{
    assert(mp);

    pthread_mutex_lock(&mp->mutex);
    ijkmp_reset_l(mp);
    pthread_mutex_unlock(&mp->mutex);
}

void ijkmp_inc_ref(IjkMediaPlayer *mp)
{
    assert(mp);
    __sync_fetch_and_add(&mp->ref_count, 1);
}

void ijkmp_dec_ref(IjkMediaPlayer **pmp)
{
    assert(pmp);
    assert(*pmp);
    IjkMediaPlayer *mp = *pmp;
    int ref_count = __sync_fetch_and_sub(&mp->ref_count, 1);
    if (ref_count == 0) {
        ijkmp_shutdown(mp);
        destroy_mp(&mp);
    }
    *pmp = NULL;
}

static int ijkmp_set_data_source_l(IjkMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);

    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    char *dup_url = strdup(url);
    if (!dup_url)
        return EIJK_OUT_OF_MEMORY;

    free(mp->data_source);
    mp->data_source = dup_url;
    mp->mp_state = MP_STATE_INITIALIZED;
    return 0;
}

int ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_set_data_source_l(mp, url);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static int ijkmp_prepare_async_l(IjkMediaPlayer *mp)
{
    assert(mp);
    assert(url);

    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    assert(mp->data_source);

    mp->mp_state = MP_STATE_ASYNC_PREPARING;
    int retval = ijkff_prepare_async_l(mp->ffplayer, mp->data_source);
    if (retval < 0) {
        mp->mp_state = MP_STATE_ERROR;
        return retval;
    }

    return 0;
}

int ijkmp_prepare_async(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_prepare_async_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static int ijkmp_start_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    int retval = ijkff_start_l(mp->ffplayer);
    if (retval < 0) {
        return retval;
    }

    if (mp->mp_state == MP_STATE_COMPLETED)
    {
        // FIXME: handle start after completed
    }

    mp->mp_state = MP_STATE_STARTED;
    return 0;
}

int ijkmp_start(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_start_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static int ijkmp_pause_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    int retval = ijkff_pause_l(mp->ffplayer);
    if (retval < 0) {
        return retval;
    }

    if (mp->mp_state == MP_STATE_STARTED)
        mp->mp_state = MP_STATE_PAUSED;

    return 0;
}

int ijkmp_pause(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_pause_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static int ijkmp_stop_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    int retval = ijkff_stop_l(mp->ffplayer);
    if (retval < 0) {
        return retval;
    }

    // FIXME: change to MP_STATE_STOPPED in read_thread() */
    mp->mp_state = MP_STATE_STOPPED;
    return 0;
}

int ijkmp_stop(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_stop_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

bool ijkmp_is_playing(IjkMediaPlayer *mp)
{
    assert(mp);
    if (mp->mp_state == MP_STATE_PREPARED ||
        mp->mp_state == MP_STATE_STARTED) {
        return true;
    }

    return false;
}

int ijkmp_seek_to_l(IjkMediaPlayer *mp, long msec)
{
    assert(mp);

    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
    // MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
    MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);

    int retval = ijkff_seek_to_l(mp->ffplayer, msec);
    if (retval < 0) {
        return retval;
    }

    return 0;
}

int ijkmp_seek_to(IjkMediaPlayer *mp, long msec)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_seek_to(mp, msec);
    pthread_mutex_unlock(&mp->mutex);

    return retval;
}

static long ijkmp_get_current_position_l(IjkMediaPlayer *mp)
{
    return ijkff_get_current_position_l(mp->ffplayer);
}

long ijkmp_get_current_position(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    long retval = ijkmp_get_current_position_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static long ijkmp_get_duration_l(IjkMediaPlayer *mp)
{
    return ijkff_get_duration_l(mp->ffplayer);
}

long ijkmp_get_duration(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_get_duration_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

void ijkmp_set_vout(IjkMediaPlayer *mp, SDL_Vout *vout)
{
    mp->ffplayer->vout = vout;
}

SDL_Vout *ijkmp_get_vout(IjkMediaPlayer *mp)
{
    return mp->ffplayer->vout;
}
