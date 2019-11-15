/*****************************************************************************
* ijkplayer_desktop.c
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

#include "ijkplayer_underdesk.h"
#include "pipeline_underdesk.h"
#include "../ijkplayer_internal.h"

#include "ijksdl/desktop/ijksdl_desktop.h"

IjkMediaPlayer *ijkmp_desktop_create(int(*msg_loop)(void *))
{
    IjkMediaPlayer *mp = ijkmp_create(msg_loop);
    if (!mp)
        goto fail;

    mp->ffplayer->vout = SDL_Vout_Callback_Create();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->ffplayer->pipeline = ffpipeline_create_desktop(mp->ffplayer);

    return mp;
fail:
    ijkmp_dec_ref_p(&mp);
    return NULL;
}

int ijkmp_set_video_callback(IjkMediaPlayer *mp, void *userdata, ijkmp_video_draw callback)
{
    if (!mp || !mp->ffplayer || !mp->ffplayer->vout)
        return -1;

    SDL_Vout *vout = mp->ffplayer->vout;
    SDL_Vout_Set_Callback(vout, userdata, callback);
    return 0;
}

