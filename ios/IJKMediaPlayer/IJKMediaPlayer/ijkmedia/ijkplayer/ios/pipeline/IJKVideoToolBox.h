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

#ifndef IJKMediaPlayer_videotoolbox_h
#define IJKMediaPlayer_videotoolbox_h

#include "ff_ffplay.h"

typedef struct Ijk_VideoToolBox_Opaque Ijk_VideoToolBox_Opaque;
typedef struct Ijk_VideoToolBox Ijk_VideoToolBox;

struct Ijk_VideoToolBox {

    Ijk_VideoToolBox_Opaque *opaque;

    int  (*decode_frame)(Ijk_VideoToolBox_Opaque *opaque);
    void (*free)(Ijk_VideoToolBox_Opaque *opaque);
};

Ijk_VideoToolBox *Ijk_VideoToolbox_Async_Create(FFPlayer* ffp, AVCodecContext* ic);
Ijk_VideoToolBox *Ijk_VideoToolbox_Sync_Create(FFPlayer* ffp, AVCodecContext* ic);

#endif
