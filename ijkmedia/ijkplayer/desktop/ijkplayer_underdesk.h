/*****************************************************************************
* ijkplayer_desktop.h
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
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

#ifndef IJKPLAYER_IJKMEDIA_IJKPLAYER_DESKUNDER_PLAYER_H
#define IJKPLAYER_IJKMEDIA_IJKPLAYER_DESKUNDER_PLAYER_H

#include "../ijkplayer.h"

IjkMediaPlayer *ijkmp_desktop_create(int(*msg_loop)(void *));

typedef int (*ijkmp_video_draw)(void *userdata, 
    int w, int h, int sar_num, int sar_den, 
    int planes, uint16_t *linesize, uint8_t **pixels);

int ijkmp_set_video_callback(IjkMediaPlayer *mp, void *userdata, ijkmp_video_draw callback);


#endif //IJKPLAYER_IJKMEDIA_IJKPLAYER_DESKUNDER_PLAYER_H