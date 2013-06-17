/*****************************************************************************
 * ijksdl_vout_android.c
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

#include "ijksdl_vout_android.h"

#include <assert.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "ijksdl_vout.h"
#include "ijksdl_vout_internal.h"

typedef struct SDL_VoutSurface_Opaque {
    SDL_Vout *vout;
} SDL_VoutSurface_Opaque;

typedef struct SDL_Vout_Opaque {
    ANativeWindow *native_window;
    SDL_VoutSurface dummy_surface;
    SDL_VoutSurface_Opaque dummy_surface_opaque;
} SDL_Vout_Opaque;

static void vout_free_l(SDL_Vout *vout)
{
    if (!vout)
        return;

    SDL_Vout_Opaque *opaque = vout->opaque;
    if (opaque) {
        if (opaque->native_window) {
            ANativeWindow_release(opaque->native_window);
        }
        free(vout->opaque);
        vout->opaque = NULL;
    }

    SDL_Vout_FreeInternal(vout);
}

static SDL_VoutSurface *vout_set_video_mode_l(SDL_Vout *vout, int w, int h, int bpp, int flags)
{
    SDL_Vout_Opaque *opaque = vout->opaque;
    SDL_VoutSurface *surface = &opaque->dummy_surface;
    ANativeWindow *native_window = opaque->native_window;
    int curr_w = 0;
    int curr_h = 0;
    int curr_format = 0;

    if (!native_window)
        return NULL;

    curr_w = ANativeWindow_getWidth(native_window);
    curr_h = ANativeWindow_getHeight(native_window);
    curr_format = ANativeWindow_getFormat(native_window);
    if (curr_w == w && curr_h == h) {
        surface->format = curr_format;
        return surface;
    }

    ANativeWindow_setBuffersGeometry(native_window, w, h, curr_format);
    curr_w = ANativeWindow_getWidth(native_window);
    curr_h = ANativeWindow_getHeight(native_window);
    curr_format = ANativeWindow_getFormat(native_window);
    if (curr_w == w && curr_h == h) {
        surface->format = curr_format;
        return surface;
    }

    return NULL;
}

static SDL_VoutSurface *vout_set_video_mode(SDL_Vout *vout, int w, int h, int bpp, Uint32 flags)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutSurface *surface = vout_set_video_mode_l(vout, w, h, bpp, flags);
    SDL_UnlockMutex(vout->mutex);
    return surface;
}

static int voud_display_overlay(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    // FIXME: implement
    return -1;
}

SDL_Vout *SDL_VoutAndroid_CreateForANativeWindow()
{
    SDL_Vout *vout = SDL_Vout_CreateInternal();
    if (!vout)
        return NULL;

    SDL_Vout_Opaque *opaque = malloc(sizeof(SDL_Vout_Opaque));
    if (!opaque)
    {
        vout_free_l(vout);
        return NULL;
    }
    memset(opaque, 0, sizeof(SDL_Vout_Opaque));

    opaque->dummy_surface_opaque.vout = vout;

    opaque->dummy_surface.opaque = &opaque->dummy_surface_opaque;

    vout->opaque = opaque;
    vout->free_l = vout_free_l;
    vout->set_video_mode = vout_set_video_mode;
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

SDL_Vout *SDL_VoutAndroid_CreateForAndroidSurface()
{
    return SDL_VoutAndroid_CreateForANativeWindow();
}

void SDL_VoutAndroid_SetAndroidSurface(SDL_Vout *vout, JNIEnv *env, jobject android_surface)
{
    if (!android_surface)
        return;

    ANativeWindow *native_window = ANativeWindow_fromSurface(env, android_surface);
    if (!native_window)
        return;

    SDL_VoutAndroid_SetNativeWindow(vout, native_window);
    ANativeWindow_release(native_window);
}
