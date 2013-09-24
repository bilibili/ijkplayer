/*****************************************************************************
 * ijksdl_vout_android_nativewindow.c
 *****************************************************************************
 *
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

#include "ijksdl_vout_android_nativewindow.h"

#include <assert.h>
#include <android/native_window.h>
#include "ijkutil/ijkutil.h"
#include "../ijksdl_vout.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_inc_ffmpeg.h"
#include "android_nativewindow.h"

typedef struct SDL_VoutSurface_Opaque {
    SDL_Vout *vout;
} SDL_VoutSurface_Opaque;

typedef struct SDL_Vout_Opaque {
    ANativeWindow *native_window;
} SDL_Vout_Opaque;

static SDL_VoutOverlay *vout_create_overlay_l(int width, int height, Uint32 format, SDL_Vout *vout)
{
    return SDL_VoutFFmpeg_CreateOverlay(width, height, format, vout);
}

static SDL_VoutOverlay *vout_create_overlay(int width, int height, Uint32 format, SDL_Vout *vout)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutOverlay *overlay = vout_create_overlay_l(width, height, format, vout);
    SDL_UnlockMutex(vout->mutex);
    return overlay;
}

static void vout_free_l(SDL_Vout *vout)
{
    if (!vout)
        return;

    SDL_Vout_Opaque *opaque = vout->opaque;
    if (opaque) {
        if (opaque->native_window) {
            ANativeWindow_release(opaque->native_window);
        }
    }

    SDL_Vout_FreeInternal(vout);
}

static int voud_display_overlay_l(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    SDL_Vout_Opaque *opaque = vout->opaque;
    ANativeWindow *native_window = opaque->native_window;

    if (!native_window) {
        ALOGE("voud_display_overlay_l: NULL native_window");
        return -1;
    }

    if (!overlay) {
        ALOGE("voud_display_overlay_l: NULL overlay");
        return -1;
    }

    if (overlay->w <= 0 || overlay->h <= 0) {
        ALOGE("voud_display_overlay_l: invalid overlay dimensions(%d, %d)", overlay->w, overlay->h);
        return -1;
    }

    return sdl_native_window_display_l(native_window, overlay);
}

static int voud_display_overlay(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    SDL_LockMutex(vout->mutex);
    int retval = voud_display_overlay_l(vout, overlay);
    SDL_UnlockMutex(vout->mutex);
    return retval;
}

SDL_Vout *SDL_VoutAndroid_CreateForANativeWindow()
{
    SDL_Vout *vout = SDL_Vout_CreateInternal(sizeof(SDL_Vout_Opaque));
    if (!vout)
        return NULL;

    SDL_Vout_Opaque *opaque = vout->opaque;
    opaque->native_window = NULL;

    vout->create_overlay = vout_create_overlay;
    vout->free_l = vout_free_l;
    vout->display_overlay = voud_display_overlay;

    return vout;
}

static void SDL_VoutAndroid_SetNativeWindow_l(SDL_Vout *vout, ANativeWindow *native_window)
{
    SDL_Vout_Opaque *opaque = vout->opaque;

    if (opaque->native_window == native_window)
        return;

    if (opaque->native_window)
        ANativeWindow_release(opaque->native_window);

    if (native_window)
        ANativeWindow_acquire(native_window);

    opaque->native_window = native_window;
}

void SDL_VoutAndroid_SetNativeWindow(SDL_Vout *vout, ANativeWindow *native_window)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutAndroid_SetNativeWindow_l(vout, native_window);
    SDL_UnlockMutex(vout->mutex);
}
