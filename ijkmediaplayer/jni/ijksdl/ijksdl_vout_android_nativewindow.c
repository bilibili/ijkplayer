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
#include "ijksdl_ffinc.h"
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
    int buf_w = (w + 1) & ~1;
    int buf_h = (h + 1) & ~1;

    assert(bpp == 0);
    if (!native_window)
        return NULL;

    curr_w = ANativeWindow_getWidth(native_window);
    curr_h = ANativeWindow_getHeight(native_window);
    curr_format = ANativeWindow_getFormat(native_window);
    if (curr_w == buf_w && curr_h == buf_h) {
        surface->w = w;
        surface->h = h;
        surface->format = curr_format;
        return surface;
    }

    ALOGI("vout_set_video_mode_l (w:%d, h:%d, fmt:%d) => (w:%d, h:%d, fmt:%d)",
        curr_w, curr_h, curr_format,
        buf_w, buf_h, curr_format);
    ANativeWindow_setBuffersGeometry(native_window, buf_w, buf_h, curr_format);
    curr_w = ANativeWindow_getWidth(native_window);
    curr_h = ANativeWindow_getHeight(native_window);
    curr_format = ANativeWindow_getFormat(native_window);
    if (curr_w == buf_w && curr_h == buf_h) {
        surface->w = w;
        surface->h = h;
        surface->format = curr_format;
        return surface;
    }

    ALOGE("vout_set_video_mode_l (w:%d, h:%d, fmt:%d) => (w:%d, h:%d, fmt:%d)",
        curr_w, curr_h, curr_format,
        buf_w, buf_h, curr_format);
    return NULL;
}

static SDL_VoutSurface *vout_set_video_mode(SDL_Vout *vout, int w, int h, int bpp, Uint32 flags)
{
    SDL_LockMutex(vout->mutex);
    SDL_VoutSurface *surface = vout_set_video_mode_l(vout, w, h, bpp, flags);
    SDL_UnlockMutex(vout->mutex);
    return surface;
}

static void vout_copy_image_yv12(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(overlay->format == SDL_YV12_OVERLAY);
    assert(overlay->planes == 3);

    int min_height = IJKMIN(out_buffer->height, overlay->h);
    int dst_y_pitch = out_buffer->stride;
    int dst_c_pitch = IJKALIGN(out_buffer->stride / 2, 16);
    int dst_y_size = dst_y_pitch * out_buffer->height;
    int dst_c_size = dst_c_pitch * out_buffer->height / 2;

    uint8_t *dst_pixels[] = {
        out_buffer->bits,
        out_buffer->bits + dst_y_size,
        out_buffer->bits + dst_y_size + dst_c_size
    };
    uint8_t *dst_planes_size[] = { dst_y_size, dst_c_size, dst_c_size };
    int dst_pitches[] = { dst_y_pitch, dst_c_pitch, dst_c_pitch };

    for (int i = 0; i < 3; ++i) {
        int dst_pitch = dst_pitches[i];
        int src_pitch = overlay->pitches[i];
        uint8_t *dst_pixels = dst_pixels[i];
        const uint8_t *src_pixels = overlay->pixels[i];
        int dst_plane_size = dst_planes_size[i];

        if (dst_pitch == src_pitch) {
            memcpy(dst_pixels, src_pixels, dst_plane_size);
        } else {
            int bytewidth = IJKMIN(dst_pitch, src_pitch);
            av_image_copy_plane(dst_pixels, dst_pitch, src_pixels, src_pitch, bytewidth, min_height);
        }
    }
}

static int voud_display_overlay_l(SDL_Vout *vout, SDL_VoutOverlay *overlay)
{
    SDL_Vout_Opaque *opaque = vout->opaque;
    ANativeWindow *native_window = opaque->native_window;
    int curr_w, curr_h, curr_format;
    int retval;

    if (!native_window)
        return -1;

    if (!overlay || overlay->w <= 0 || overlay->h <= 0)
        return -1;

    int buf_w = (overlay->w + 1) & ~1;
    int buf_h = (overlay->h + 1) & ~1;

    curr_w = ANativeWindow_getWidth(native_window);
    curr_h = ANativeWindow_getHeight(native_window);
    curr_format = ANativeWindow_getFormat(native_window);
    if (curr_w != buf_w ||
        curr_h != buf_h ||
        curr_format != overlay->format) {

        // correct w, h, format
        ALOGI("vout_set_video_mode_l (w:%d, h:%d, fmt:%d) => (w:%d, h:%d, fmt:%d)",
            curr_w, curr_h, curr_format,
            buf_w, buf_h, overlay->format);
        ANativeWindow_setBuffersGeometry(native_window, buf_w, buf_h, overlay->format);
        curr_w = ANativeWindow_getWidth(native_window);
        curr_h = ANativeWindow_getHeight(native_window);
        curr_format = ANativeWindow_getFormat(native_window);

        if (curr_w != buf_w ||
            curr_h != buf_h ||
            curr_format != overlay->format) {
            ALOGE("unexpected native window (w:%d, h:%d, fmt:%d), expecting (w:%d, h:%d, fmt:%d)",
                curr_w, curr_h, curr_format,
                buf_w, buf_h, overlay->format);
            return -1;
        }
    }

    ANativeWindow_Buffer out_buffer;
    retval = ANativeWindow_lock(native_window, &out_buffer, NULL);
    if (retval < 0) {
        ALOGE("voud_display_overlay_l: ANativeWindow_lock: failed %d", retval);
        return retval;
    }

    if (out_buffer.width != buf_w || out_buffer.height != buf_w) {
        ALOGE("unexpected native window buffer (w:%d, h:%d, fmt:%d), expecting (w:%d, h:%d, fmt:%d)",
            out_buffer.width, out_buffer.height, out_buffer.format,
            buf_w, buf_h, overlay->format);
        return -1;
    }

    int copy_ret = 0;
    switch (out_buffer.format) {
    case SDL_YV12_OVERLAY:
        vout_copy_image_yv12(&out_buffer, overlay);
        break;
    default:
        ALOGE("voud_display_overlay_l: unexpected buffer format: %d", out_buffer.format);
        copy_ret = -1;
        break;
    }

    retval = ANativeWindow_unlockAndPost(native_window);
    if (retval < 0) {
        ALOGE("voud_display_overlay_l: ANativeWindow_unlockAndPost: failed %d", retval);
        return retval;
    }
    return copy_ret;
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
    opaque->dummy_surface_opaque.vout = vout;
    opaque->dummy_surface.opaque = &opaque->dummy_surface_opaque;

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
