/*
 * ff_ffplay.h
 *
 * Copyright (c) 2003 Fabrice Bellard
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

#ifndef IJKPLAYER__FF_FFPLAY_H
#define IJKPLAYER__FF_FFPLAY_H

#include "ff_ffplay_def.h"

void ijkff_global_init();
void ijkff_global_uninit();

void ijkff_destroy_ffplayer(FFPlayer **pffp);

int ijkff_stream_open_l(FFPlayer *ffp, const char *file_name);
int ijkff_start_l(FFPlayer *ffp);
int ijkff_pause_l(FFPlayer *ffp);
int ijkff_stop_l(FFPlayer *ffp);
int ijkff_wait_stop(FFPlayer *ffp);

#endif
