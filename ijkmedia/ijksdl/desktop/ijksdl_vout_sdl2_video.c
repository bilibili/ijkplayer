/*****************************************************************************
* ijksdl_vout_callback.c
*****************************************************************************
*
* copyright (c) 2020 befovy <befovy@gmail.com>
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

#include "ijksdl_vout_sdl2_video.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_vout_overlay_ffmpeg.h"

#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL.h>
#include <assert.h>


struct SDL_Vout_Opaque {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Uint32 format;
    int width;
    int height;
};

static SDL_Class sdl2_vout_class = {.name = "sdl2_vout_class"};

static SDL_VoutOverlay *func_create_overlay(int width, int height, int frame_format, SDL_Vout *vout) {
    SDL_LockMutex(vout->mutex);
    SDL_VoutOverlay *overlay = SDL_VoutFFmpeg_CreateOverlay(width, height, frame_format, vout);
    SDL_UnlockMutex(vout->mutex);
    return overlay;
}


static int sdl2_display(SDL_Vout_Opaque *opaque, SDL_VoutOverlay *overlay) {
    if (opaque->renderer == NULL) {
        SDL_Window *window = opaque->window;
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            renderer = SDL_CreateRenderer(window, -1, 0);
        }
        if (renderer == NULL) {
            ALOGE("SDL2 2 Error : %s", SDL_GetError());
        }
        opaque->renderer = renderer;
    }

    SDL_Renderer *renderer = opaque->renderer;

    assert(overlay->format == SDL_FCC_I420);

    Uint32 format = 0;
    int access = 0, w = 0, h = 0, ret = 0;
    if (opaque->texture != NULL)
        ret = SDL_QueryTexture(opaque->texture, &format, &access, &w, &h);
    if (ret < 0 || format != opaque->format || w != overlay->w || h != overlay->h) {
        if (opaque->texture)
            SDL_DestroyTexture(opaque->texture);
        opaque->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, overlay->w,
                                            overlay->h);
        opaque->format = SDL_PIXELFORMAT_IYUV;
    }
    if (opaque->texture == NULL) {
        return -1;
    }
    SDL_Texture *texture = opaque->texture;
    SDL_UpdateYUVTexture(texture, NULL,
                         overlay->pixels[0], overlay->pitches[0],
                         overlay->pixels[1], overlay->pitches[1],
                         overlay->pixels[2], overlay->pitches[2]);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    return 0;
}

static int func_display_overlay(SDL_Vout *vout, SDL_VoutOverlay *overlay) {
    SDL_LockMutex(vout->mutex);

    SDL_Vout_Opaque *opaque = vout->opaque;
    if (!opaque || !opaque->window) {
        return -1;
    }
    sdl2_display(opaque, overlay);
    SDL_UnlockMutex(vout->mutex);
    return 0;
}

static void func_free_l(SDL_Vout *vout) {
    if (!vout)
        return;
    SDL_Vout_Opaque *opaque = vout->opaque;
    if (opaque) {
        if (opaque->texture) {
            SDL_DestroyTexture(opaque->texture);
            opaque->texture = NULL;
        }
        if (opaque->renderer) {
            SDL_DestroyRenderer(opaque->renderer);
            opaque->renderer = NULL;
        }
    }
    SDL_Vout_FreeInternal(vout);
}

// https://github.com/vheuken/SDL-Render-Thread-Example/blob/master/main.cpp
static int set_window(SDL_Vout *vout, void *window) {
    if (!vout || !vout->opaque)
        return -1;
    SDL_LockMutex(vout->mutex);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    vout->opaque->window = window;
    //vout->opaque->context = SDL_GL_GetCurrentContext();
    //SDL_GL_MakeCurrent(window, NULL);
    SDL_Vout_Opaque *opaque = vout->opaque;
    if (opaque->renderer != NULL) {
        SDL_DestroyRenderer(opaque->renderer);
        opaque->renderer = NULL;
    }
    if (opaque->renderer == NULL && window != NULL) {
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            renderer = SDL_CreateRenderer(window, -1, 0);
        }
        if (renderer == NULL) {
            ALOGE("SDL2 create renderer failed: %s", SDL_GetError());
        }
        opaque->renderer = renderer;
    }
    SDL_UnlockMutex(vout->mutex);
    return 0;
}

SDL_Vout *SDL_Vout_sdl2_Create() {
    SDL_Vout *vout = SDL_Vout_CreateInternal(sizeof(SDL_Vout_Opaque));
    if (!vout)
        return NULL;
    vout->opaque_class = &sdl2_vout_class;
    vout->overlay_format = SDL_FCC_I420;
    vout->create_overlay = func_create_overlay;
    vout->display_overlay = func_display_overlay;
    vout->set_window = set_window;
    vout->free_l = func_free_l;
    return vout;
}
