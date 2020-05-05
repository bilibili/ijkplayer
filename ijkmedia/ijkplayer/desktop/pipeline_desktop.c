/*****************************************************************************
* pipeline_desktop.c
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

#include "pipeline_desktop.h"

#include "../pipeline/ffpipeline_ffplay.h"
#include "ijksdl/desktop/ijksdl_desktop.h"
#include "ijksdl/dummy/ijksdl_dummy.h"
#include "../ijkplayer_internal.h"

IjkMediaPlayer *ijkmp_desktop_create(int(*msg_loop)(void *), int vout_type)
{
    IjkMediaPlayer *mp = ijkmp_create(msg_loop);
    if (!mp)
        goto fail;

    switch (vout_type) {
        case IJKFF_VOUT_SDL2:
            mp->ffplayer->vout = SDL_Vout_sdl2_Create();
            break;
        case IJKFF_VOUT_CALLBACK:
            mp->ffplayer->vout = SDL_Vout_Callback_Create();
            break;
        case IJKFF_VOUT_DUMMY:
            mp->ffplayer->vout = SDL_VoutDummy_Create();
            break;
#if IJK_GLFW
        case IJKFF_VOUT_GLFW:
            mp->ffplayer->vout = SDL_Vout_glfw_Create();
            break;
#endif
        default:
            break;
    }
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

int ijkmp_set_window(IjkMediaPlayer *mp, void *window)
{
    if (!mp || !mp->ffplayer || !mp->ffplayer->vout)
        return -1;
    SDL_Vout *vout = mp->ffplayer->vout;

    SDL_VoutSetWindow(vout, window);
    return 0;
}

static SDL_Aout *func_open_audio_output_l(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    SDL_Aout *aout = NULL;
    if (ffp->aout_type == 0)
        aout = SDL_Aout_SDL2_Audio_Create();
    else if (ffp->aout_type == 1)
        aout = SDL_Aout_Port_Audio_Create();
    return aout;
}


IJKFF_Pipeline *ffpipeline_create_desktop(FFPlayer *ffp)
{
    IJKFF_Pipeline *pipeline = ffpipeline_create_from_ffplay(ffp);
    if (pipeline) {
        pipeline->func_open_audio_output = func_open_audio_output_l;
    }
    return pipeline;
}
