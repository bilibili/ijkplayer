/*****************************************************************************
* ijksdl_vout_callback.c
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

#ifndef IJKPLAYER_IJKMEDIA_IJKSDL_DESKTOP_VOUT_CALLBACK_H
#define IJKPLAYER_IJKMEDIA_IJKSDL_DESKTOP_VOUT_CALLBACK_H

#include "ijksdl_vout_uni_callback.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_vout_overlay_ffmpeg.h"

struct SDL_Vout_Opaque {
    int frame_format;
    ijkmp_vout_callback cb;
    void *cb_data;
};

static SDL_Class uni_vout_class = { .name = "uni_vout" };


int SDL_Vout_Set_Callback(SDL_Vout *vout, void *userdata, ijkmp_vout_callback callback)
{
    if (vout && vout->opaque_class == &uni_vout_class) {
        SDL_LockMutex(vout->mutex);
        SDL_Vout_Opaque *opaque = vout->opaque;
        opaque->cb_data = userdata;
        opaque->cb = callback;
        SDL_UnlockMutex(vout->mutex);
    }
    return 0;
}

static int func_display_overlay(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    SDL_LockMutex(vout->mutex);

    SDL_Vout_Opaque *opaque = vout->opaque;
    int ret = -1;
    if (opaque->cb) {
        ret = opaque->cb(opaque->cb_data, overlay->w, overlay->h, 
            overlay->sar_num, overlay->sar_den, 
            overlay->planes, overlay->pitches, overlay->pixels);
    }
    SDL_UnlockMutex(vout->mutex);
    return ret;
}


static SDL_VoutOverlay *func_create_overlay(int width, int height, int frame_format, SDL_Vout *vout)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutOverlay *overlay = SDL_VoutFFmpeg_CreateOverlay(width, height, frame_format, vout);
    SDL_UnlockMutex(vout->mutex);
    return overlay;
}


static void func_free_l(SDL_Vout *vout)
{
    if (!vout)
        return;

    SDL_Vout_FreeInternal(vout);
}



SDL_Vout *SDL_Vout_Callback_Create()
{
    SDL_Vout *vout = SDL_Vout_CreateInternal(sizeof(SDL_Vout_Opaque));
    vout->opaque_class = &uni_vout_class;
    vout->overlay_format = SDL_FCC_RGBA;
    vout->create_overlay = func_create_overlay;
    vout->display_overlay = func_display_overlay;
    vout->free_l = func_free_l;
    return vout;
}

#endif // IJKPLAYER_IJKMEDIA_IJKSDL_DESKTOP_VOUT_CALLBACK_H