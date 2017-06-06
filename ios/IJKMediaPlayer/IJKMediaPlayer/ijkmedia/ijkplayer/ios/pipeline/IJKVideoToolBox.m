/*****************************************************************************
 * IJKVideoToolBox.m
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

#include "IJKVideoToolBox.h"
#include "ijksdl/ijksdl_inc_internal.h"
#include "IJKVideoToolBoxAsync.h"
#include "IJKVideoToolBoxSync.h"

inline static Ijk_VideoToolBox *Ijk_VideoToolbox_CreateInternal(int async, FFPlayer* ffp, AVCodecContext* ic)
{
    Ijk_VideoToolBox *vtb = (Ijk_VideoToolBox*) mallocz(sizeof(Ijk_VideoToolBox));
    if (!vtb)
        return NULL;
    if (async) {
        vtb->opaque = videotoolbox_async_create(ffp, ic);
        vtb->decode_frame = videotoolbox_async_decode_frame;
        vtb->free = videotoolbox_async_free;
    } else {
        vtb->opaque = videotoolbox_sync_create(ffp, ic);
        vtb->decode_frame = videotoolbox_sync_decode_frame;
        vtb->free = videotoolbox_sync_free;
    }

    if (!vtb->opaque) {
        freep((void **)&vtb);
        return NULL;
    }
    return vtb;
}

Ijk_VideoToolBox *Ijk_VideoToolbox_Async_Create(FFPlayer* ffp, AVCodecContext* ic) {
    return Ijk_VideoToolbox_CreateInternal(1, ffp, ic);
}

Ijk_VideoToolBox *Ijk_VideoToolbox_Sync_Create(FFPlayer* ffp, AVCodecContext* ic) {
    return Ijk_VideoToolbox_CreateInternal(0, ffp, ic);
}
