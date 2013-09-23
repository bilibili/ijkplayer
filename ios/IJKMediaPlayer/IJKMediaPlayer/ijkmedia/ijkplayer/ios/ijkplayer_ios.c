/*
 * ijkplayer_ios.c
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

#include <stdio.h>

#include "ijkplayer_ios.h"

#include <assert.h>
#include "ijksdl/ios/ijksdl_ios.h"
#include "ijksdl/dummy/ijksdl_dummy.h"
#include "ijkplayer/ff_fferror.h"
#include "ijkplayer/ff_ffplay.h"
#include "ijkplayer/ijkplayer_internal.h"

IjkMediaPlayer *ijkmp_ios_create(void *(*msg_loop)(void*))
{
    IjkMediaPlayer *mp = ijkmp_create(msg_loop);
    if (!mp)
        goto fail;

    mp->ffplayer->vout = SDL_VoutDummy_Create();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->ffplayer->aout = SDL_AoutIos_CreateForAudioUnit();
    if (!mp->ffplayer->vout)
        goto fail;

    return mp;

fail:
    ijkmp_dec_ref_p(&mp);
    return NULL;
}
