/*****************************************************************************
* ijksdl_desktop_vout_glfw.c
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



#include "ijksdl_desktop_vout_glfw.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_vout_overlay_ffmpeg.h"

#include "ijksdl/gles2/internal.h"
#include "ijksdl_desktop_gl.h"

struct SDL_Vout_Opaque {
    IJK_GL *gl;
    GLFWwindow *window;
};

static SDL_Class glfw_vout_class = {.name = "glfw_vout"};

static SDL_VoutOverlay *func_create_overlay(int width, int height, int frame_format, SDL_Vout *vout) {
    SDL_LockMutex(vout->mutex);
    SDL_VoutOverlay *overlay = SDL_VoutFFmpeg_CreateOverlay(width, height, frame_format, vout);
    SDL_UnlockMutex(vout->mutex);
    return overlay;
}


static int func_display_overlay(SDL_Vout *vout, SDL_VoutOverlay *overlay) {
    SDL_LockMutex(vout->mutex);

    SDL_Vout_Opaque *opaque = vout->opaque;
    if (!opaque || !opaque->window) {
        return -1;
    }
    IJK_GL_display(opaque->gl, opaque->window, overlay);
    SDL_UnlockMutex(vout->mutex);
    return 0;
}

static void func_free_l(SDL_Vout *vout) {
    if (!vout)
        return;
    SDL_Vout_Opaque *opaque = vout->opaque;
    if (opaque) {
        if (opaque->gl) {
            IJK_GL_free(opaque->gl);
            opaque->gl = NULL;
        }
    }
    SDL_Vout_FreeInternal(vout);
}


static int set_window(SDL_Vout *vout, void *window) {
    SDL_LockMutex(vout->mutex);
    SDL_Vout_Opaque *opaque = vout->opaque;
    //IJK_GL_terminate(opaque->gl);
    opaque->window = window;
    SDL_UnlockMutex(vout->mutex);
    return 0;
}

SDL_Vout *SDL_Vout_glfw_Create() {
    SDL_Vout *vout = SDL_Vout_CreateInternal(sizeof(SDL_Vout_Opaque));
    if (!vout)
        return NULL;

    SDL_Vout_Opaque *opaque = vout->opaque;
    opaque->gl = IJK_GL_create();
    if (!opaque->gl) {
        func_free_l(vout);
        return NULL;
    }
    vout->opaque_class = &glfw_vout_class;
    vout->overlay_format = SDL_FCC_I420;
    vout->create_overlay = func_create_overlay;
    vout->display_overlay = func_display_overlay;
    vout->set_window = set_window;
    vout->free_l = func_free_l;
    return vout;
}