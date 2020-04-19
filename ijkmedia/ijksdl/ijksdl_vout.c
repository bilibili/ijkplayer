/*****************************************************************************
 * ijksdl_vout.c
 *****************************************************************************
 *
 * Copyright (c) 2013 Bilibili
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_vout.h"
#include "ijksdl_gles2.h"
#include <stdlib.h>

#include <assert.h>
#if defined(__ANDROID__)
#include <android/native_window_jni.h>
#endif

void SDL_VoutFree(SDL_Vout *vout)
{
    if (!vout)
        return;

    if (vout->free_l) {
        vout->free_l(vout);
    } else {
        free(vout);
    }
}

void SDL_VoutFreeP(SDL_Vout **pvout)
{
    if (!pvout)
        return;

    SDL_VoutFree(*pvout);
    *pvout = NULL;
}

int SDL_VoutDisplayYUVOverlay(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    if (vout && overlay && vout->display_overlay)
        return vout->display_overlay(vout, overlay);

    return -1;
}

int SDL_VoutSetOverlayFormat(SDL_Vout *vout, Uint32 overlay_format, int vout_type)
{
    if (!vout)
        return -1;

    vout->overlay_format = overlay_format;
    vout->vout_type = vout_type;
    return 0;
}

int SDL_VoutSetWindow(SDL_Vout *vout, void *window)
{
    if(vout && vout->set_window)
        return vout->set_window(vout, window);
    return -1;
}

int SDL_Vout_TakeSnapShot(SDL_Vout *vout, void *opaque, IJK_GLES2_Renderer_funcGetSnapShot get_snap_shot)
{
    if (!vout || !vout->get_renderer)
        return -1;

    IJK_GLES2_Renderer *renderer = vout->get_renderer(vout);
    if (!renderer)
        return -1;

    return IJK_GLES2_Renderer_snapShot(renderer, opaque, get_snap_shot) ? 0 : -1;
}


SDL_VoutOverlay *SDL_Vout_CreateOverlay(int width, int height, int frame_format, SDL_Vout *vout)
{
    if (vout && vout->create_overlay)
        return vout->create_overlay(width, height, frame_format, vout);

    return NULL;
}

int SDL_VoutLockYUVOverlay(SDL_VoutOverlay *overlay)
{
    if (overlay && overlay->lock)
        return overlay->lock(overlay);

    return -1;
}

int SDL_VoutUnlockYUVOverlay(SDL_VoutOverlay *overlay)
{
    if (overlay && overlay->unlock)
        return overlay->unlock(overlay);

    return -1;
}

void SDL_VoutFreeYUVOverlay(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return;

    if (overlay->free_l) {
        overlay->free_l(overlay);
    } else {
        free(overlay);
    }
}

void SDL_VoutUnrefYUVOverlay(SDL_VoutOverlay *overlay)
{
    if (overlay && overlay->unref)
        overlay->unref(overlay);
}

int SDL_VoutFillFrameYUVOverlay(SDL_VoutOverlay *overlay, const AVFrame *frame)
{
    if (!overlay || !overlay->func_fill_frame)
        return -1;

    return overlay->func_fill_frame(overlay, frame);
}
