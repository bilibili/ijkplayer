/*****************************************************************************
 * ijkplayer.h
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

#ifndef IJKPLAYER_H
#define IJKPLAYER_H

#include <stdbool.h>

struct IjkMediaPlayer;

void ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url);
void ijkmp_prepare_async(IjkMediaPlayer *mp);
void ijkmp_start(IjkMediaPlayer *mp);
void ijkmp_stop(IjkMediaPlayer *mp);
int  ijkmp_get_video_width(IjkMediaPlayer *mp);
int  ijkmp_get_video_height(IjkMediaPlayer *mp);
void ijkmp_seek_to(IjkMediaPlayer *mp, int msec);
void ijkmp_pause(IjkMediaPlayer *mp);
bool ijkmp_is_playing(IjkMediaPlayer *mp);
int  ijkmp_get_current_position(IjkMediaPlayer *mp);
int  ijkmp_get_duration(IjkMediaPlayer *mp);
void ijkmp_release(IjkMediaPlayer *mp);
void ijkmp_reset(IjkMediaPlayer *mp);

// android api
void ijkmp_set_video_surface(IjkMediaPlayer *mp, void *surface);

#endif
