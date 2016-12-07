/*****************************************************************************
 * IJKVideoToolBox.h
 *****************************************************************************
 *
 * copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
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

#ifndef IJKMediaPlayer_videotoolbox_sync_h
#define IJKMediaPlayer_videotoolbox_sync_h

#include "ff_ffplay.h"

typedef struct Ijk_VideoToolBox_Opaque Ijk_VideoToolBox_Opaque;

Ijk_VideoToolBox_Opaque* videotoolbox_sync_create(FFPlayer* ffp, AVCodecContext* ic);

int videotoolbox_sync_decode_frame(Ijk_VideoToolBox_Opaque* opaque);

void videotoolbox_sync_free(Ijk_VideoToolBox_Opaque* opaque);

#endif
