/*
 * ijkplayer.h
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

#ifndef IJKPLAYER__IJKPLAYER_H
#define IJKPLAYER__IJKPLAYER_H

#include <stdbool.h>

struct IjkMediaPlayer;

typedef struct IjkMediaPlayer {
    volatile int ref_count;
    void *ffplayer;
} IjkMediaPlayer;

// ref_count is 0 after open
IjkMediaPlayer *ijkmp_create();

void ijkmp_global_init();
void ijkmp_global_uninit();

// preferred to be called explicity, can be called multiple times
// NOTE: ijkmp_shutdown may block thread
void ijkmp_shutdown(IjkMediaPlayer *mp);

void ijkmp_inc_ref(IjkMediaPlayer *mp);

// call close at last release, also free memory
// NOTE: ijkmp_dec_ref may block thread
void ijkmp_dec_ref(IjkMediaPlayer **pmp);

void ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url);
void ijkmp_prepare_async(IjkMediaPlayer *mp);
void ijkmp_start(IjkMediaPlayer *mp);
void ijkmp_stop(IjkMediaPlayer *mp);
void ijkmp_pause(IjkMediaPlayer *mp);
int ijkmp_get_video_width(IjkMediaPlayer *mp);
int ijkmp_get_video_height(IjkMediaPlayer *mp);
void ijkmp_seek_to(IjkMediaPlayer *mp, int msec);
bool ijkmp_is_playing(IjkMediaPlayer *mp);
int ijkmp_get_current_position(IjkMediaPlayer *mp);
int ijkmp_get_duration(IjkMediaPlayer *mp);
void ijkmp_reset(IjkMediaPlayer *mp);

// android api
void ijkmp_set_video_surface(IjkMediaPlayer *mp, void *surface);

#endif
