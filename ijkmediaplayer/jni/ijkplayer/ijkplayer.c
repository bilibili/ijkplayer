/*
 * ijkplayer.c
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

#include "ijkplayer.h"

#include <assert.h>
#include "ijkerror.h"
#include "ff_ffplay.h"

#define MPST_CHECK_NOT_RET_INT(real, expected, errcode) \
    do { \
        if (real == expected) return errcode; \
    } while(0)

#define MPST_CHECK_NOT_RET(real, expected) \
    MPST_CHECK_NOT_RET_INT(real, expected, EIJK_INVALID_STATE)

typedef struct IjkMediaPlayer {
    volatile int ref_count;
    FFPlayer *ffplayer;

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

    ijkff_destroy_ffplayer(&mp->ffplayer);
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

IjkMediaPlayer *ijkmp_create()
{
    IjkMediaPlayer *mp = (IjkMediaPlayer *) malloc(sizeof(IjkMediaPlayer));
    if (!mp) {
        return NULL;
    }
    memset(mp, 0, sizeof(IjkMediaPlayer));

    mp->ffplayer = (FFPlayer*) malloc(sizeof(FFPlayer));
    if (!mp) {
        free(mp);
        return NULL;
    }
    memset(mp->ffplayer, 0, sizeof(FFPlayer));

    FFPlayer *ffp = mp->ffplayer;
    ijkff_reset(ffp);

    ijkmp_inc_ref(mp);
    return mp;
}

void ijkmp_shutdown(IjkMediaPlayer *mp)
{
    assert(mp);
// FIXME: implement
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

int ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url)
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

int ijkmp_prepare_async(IjkMediaPlayer *mp)
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

    int retval = ijkff_stream_open(mp->ffplayer, mp->data_source);
    if (retval < 0) {
        mp->mp_state = MP_STATE_ERROR;
        return retval;
    }

    mp->mp_state = MP_STATE_ASYNC_PREPARING;
    return 0;
}

int ijkmp_start(IjkMediaPlayer *mp)
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

    assert(mp->data_source);

    return 0;
}

void ijkmp_pause(IjkMediaPlayer *mp)
{
// FIXME: implement
}

void ijkmp_stop(IjkMediaPlayer *mp)
{
// FIXME: implement
}

int ijkmp_get_video_width(IjkMediaPlayer *mp)
{
// FIXME: implement
    return 0;
}

int ijkmp_get_video_height(IjkMediaPlayer *mp)
{
// FIXME: implement
    return 0;
}

void ijkmp_seek_to(IjkMediaPlayer *mp, int msec)
{
// FIXME: implement
}

bool ijkmp_is_playing(IjkMediaPlayer *mp)
{
// FIXME: implement
    return false;
}

int ijkmp_get_current_position(IjkMediaPlayer *mp)
{
// FIXME: implement
    return 0;
}

int ijkmp_get_duration(IjkMediaPlayer *mp)
{
// FIXME: implement
    return 0;
}

void ijkmp_reset(IjkMediaPlayer *mp)
{
// FIXME: implement
}

void ijkmp_set_vout(IjkMediaPlayer *mp, SDL_Vout *vout)
{
    mp->ffplayer->vout = vout;
}

SDL_Vout *ijkmp_get_vout(IjkMediaPlayer *mp)
{
    return mp->ffplayer->vout;
}
