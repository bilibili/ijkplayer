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

typedef struct SDL_Vout_Opaque {
    ANativeWindow *native_window;
} SDL_Vout_Opaque;

typedef struct SDL_VoutSurface_Opaque {
    ANativeWindow *native_window;
} SDL_VoutSurface_Opaque;

static void surface_opaque_free(SDL_VoutSurface *surface)
{
    if (!surface)
        return;

    SDL_VoutSurface_Opaque *opaque = surface->opaque;
    if (opaque) {
        if (opaque->native_window) {
            ANativeWindow_release(opaque->native_window);
        }
        free(opaque);
        surface->opaque = NULL;
    }

    SDL_Vout_FreeSurfaceInternal(surface);
}

static SDL_VoutSurface *surface_create_l(ANativeWindow *native_window)
{
    SDL_VoutSurface *surface = SDL_Vout_CreateSurfaceInternal();
    if (!surface)
        return NULL;

    SDL_VoutSurface_Opaque *opaque = (SDL_VoutSurface_Opaque*) malloc(sizeof(SDL_VoutSurface_Opaque));
    if (!opaque) {
        free(surface);
        return NULL;
    }
    memset(opaque, 0, sizeof(opaque));

    ANativeWindow_acquire(native_window);
    opaque->native_window = native_window;

    surface->opaque = opaque;
    surface->free_l = surface_opaque_free;
    return surface;
}

static void vout_opaque_free(SDL_Vout *vout)
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

static int vout_get_surface(SDL_Vout *vout, SDL_VoutSurface** ppsurface, int w, int h, int format)
{
    SDL_Vout_Opaque *vout_opaque = vout->opaque;
    SDL_VoutSurface *surface = *ppsurface;
    SDL_VoutSurface_Opaque *surface_opaque = NULL;
    ANativeWindow *new_native_window = NULL;

    SDL_LockMutex(vout->mutex);
    if (surface != NULL) {
        surface_opaque = surface->opaque;

        if (surface_opaque->native_window == vout_opaque->native_window &&
            w == surface->w && h == surface->h && format == surface->format) {
        } else {
            SDL_Vout_FreeSurface(surface);
            *ppsurface = NULL;
            surface = NULL;
        }
    }

    // aquire new native_window from vout */
    if (surface == NULL) {
        new_native_window = vout_opaque->native_window;
        ANativeWindow_acquire(new_native_window);
    }

    SDL_UnlockMutex(vout->mutex);

    // setup surface without lock
    if (surface == NULL) {
        if (ANativeWindow_setBuffersGeometry(new_native_window, w, h, format) < 0) {
            ANativeWindow_release(new_native_window);
            new_native_window = NULL;
        }

        if (new_native_window) {
            assert(surface);
            surface = surface_create_l(new_native_window);
            if (surface)
                new_native_window = NULL;

            ANativeWindow_release(vout_opaque->native_window);
        }

        if (new_native_window) {
            ANativeWindow_release(new_native_window);
            new_native_window = NULL;
        }
    }

    return surface ? 0 : -1;
}

SDL_Vout *SDL_VoutAndroid_CreateForANativeWindow()
{
    SDL_Vout *vout = SDL_Vout_CreateInternal();
    if (!vout)
        return NULL;

    SDL_Vout_Opaque *opaque = malloc(sizeof(SDL_Vout_Opaque));
    if (!opaque)
    {
        SDL_Vout_Free(vout);
        return NULL;
    }
    memset(opaque, 0, sizeof(SDL_Vout_Opaque));

    vout->opaque = opaque;
    vout->free_l = vout_opaque_free;
    vout->get_surface = vout_get_surface;

    return vout;
}

static void SDL_VoutAndroid_SetNativeWindow_l(SDL_Vout *vout, ANativeWindow *native_window)
{
    SDL_Vout_Opaque *opaque = vout->opaque;

    if (opaque->native_window == native_window)
        return;

    if (opaque->native_window)
        ANativeWindow_release(opaque->native_window);

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
