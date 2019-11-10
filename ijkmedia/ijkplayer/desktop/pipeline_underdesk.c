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

#include "pipeline_underdesk.h"

#include "../pipeline/ffpipeline_ffplay.h"
#include "ijksdl/desktop/ijksdl_aout_port_audio.h"

static SDL_Aout *func_open_audio_output_l(IJKFF_Pipeline *pipeline, FFPlayer *ffp)
{
    SDL_Aout *aout = SDL_Aout_Port_Audio_Create();
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