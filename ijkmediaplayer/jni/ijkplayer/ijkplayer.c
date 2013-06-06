/*****************************************************************************
 * ijkplayer.c
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

#include "ijkplayer.h"

#include "ffplay.h"

void ijkmp_init(IjkMediaPlayer *mp)
{
    ijkmp_destroy(mp);
    mp->ffplayer = malloc(sizeof(FFPlayer));
}

void ijkmp_destroy(IjkMediaPlayer *mp)
{
    if (!mp)
        return;

    // FIXME: implement
    free(mp->ffplayer);
    memset(mp, 0, sizeof(mp));
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
