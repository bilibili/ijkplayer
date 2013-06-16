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

    FFPlayer *ffp = &mp->ffplayer;
    ijkff_reset(ffp);

    ijkmp_inc_ref(mp);
    return mp;
}

void ijkmp_shutdown(IjkMediaPlayer *mp)
{
    assert(mp);
    // FIXME: implement
}

inline static void ijkmp_free(IjkMediaPlayer *mp)
{
    memset(mp->ffplayer, 0, sizeof(FFPlayer));
    free(mp->ffplayer);

    memset(mp, 0, sizeof(IjkMediaPlayer));
    free(mp);
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
        ijkmp_free(mp);
    }
    *pmp = NULL;
}

void ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url)
{
    // FIXME: implement
}

// FIXME: android spec
void ijkmp_set_video_surface(IjkMediaPlayer *mp, void *surface)
{
    // FIXME: implement
}

void ijkmp_prepare_async(IjkMediaPlayer *mp)
{
    // FIXME: implement
}

void ijkmp_start(IjkMediaPlayer *mp)
{
    // FIXME: implement
}

void ijkmp_stop(IjkMediaPlayer *mp)
{
    // FIXME: implement
}

void ijkmp_pause(IjkMediaPlayer *mp)
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
