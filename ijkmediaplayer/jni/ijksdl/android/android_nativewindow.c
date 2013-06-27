/*****************************************************************************
 * android_nativewindow_yv12.c
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

#include "android_nativewindow.h"

#include <assert.h>
#include <android/native_window.h>
#include "ijkutil/ijkutil.h"
#include "../ijksdl_vout.h"
#include "../ijksdl_vout_internal.h"
#include "../ffmpeg/ijksdl_inc_ffmpeg.h"
#include "ijksdl_inc_internal_android.h"

static int android_render_yv12_on_yv12(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    // SDLTRACE("SDL_VoutAndroid: android_render_yv12_on_yv12(%p)", overlay);
    assert(overlay->format == SDL_FCC_YV12);
    assert(overlay->planes == 3);

    int min_height = IJKMIN(out_buffer->height, overlay->h);
    int dst_y_stride = out_buffer->stride;
    int dst_c_stride = IJKALIGN(out_buffer->stride / 2, 16);
    int dst_y_size = dst_y_stride * out_buffer->height;
    int dst_c_size = dst_c_stride * out_buffer->height / 2;

    // ALOGE("stride:%d/%d, size:%d/%d", dst_y_stride, dst_c_stride, dst_y_size, dst_c_size);

    uint8_t *dst_pixels_array[] = {
        out_buffer->bits,
        out_buffer->bits + dst_y_size,
        out_buffer->bits + dst_y_size + dst_c_size,
    };
    int dst_line_height[] = { min_height, min_height / 2, min_height / 2 };
    int dst_line_size_array[] = { dst_y_stride, dst_c_stride, dst_c_stride };

    for (int i = 0; i < 3; ++i) {
        int dst_line_size = dst_line_size_array[i];
        int src_line_size = overlay->pitches[i];
        int line_height = dst_line_height[i];
        uint8_t *dst_pixels = dst_pixels_array[i];
        const uint8_t *src_pixels = overlay->pixels[i];

        if (dst_line_size == src_line_size) {
            int plane_size = src_line_size * min_height;

            // ALOGE("sdl_image_copy_plane %p %p %d", dst_pixels, src_pixels, dst_plane_size);
            memcpy(dst_pixels, src_pixels, plane_size);
        } else {
            // TODO: padding
            int bytewidth = IJKMIN(dst_line_size, src_line_size);

            // ALOGE("av_image_copy_plane %p %d %p %d %d %d", dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
            av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, line_height);
        }
    }

    return 0;
}

static int android_render_on_yv12(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_YV12: {
        return android_render_yv12_on_yv12(out_buffer, overlay);
    }
    }

    return -1;
}

static int android_render_rgb_on_rgb(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay, int bpp)
{
    // SDLTRACE("SDL_VoutAndroid: android_render_rgb_on_rgb(%p)", overlay);
    assert(overlay->format == SDL_FCC_RGBP);
    assert(overlay->planes == 1);

    int min_height = IJKMIN(out_buffer->height, overlay->h);
    int dst_stride = out_buffer->stride;
    int src_line_size = overlay->pitches[0];
    int dst_line_size = dst_stride * bpp / 8;

    uint8_t *dst_pixels = out_buffer->bits;
    const uint8_t *src_pixels = overlay->pixels[0];

    if (dst_line_size == src_line_size) {
        int plane_size = src_line_size * min_height;
        // ALOGE("android_render_rgb_on_rgb %p %p %d", dst_pixels, src_pixels, plane_size);
        memcpy(dst_pixels, src_pixels, plane_size);
    } else {
        // TODO: padding
        int bytewidth = IJKMIN(dst_line_size, src_line_size);

        // ALOGE("android_render_rgb_on_rgb %p %d %p %d %d %d", dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
        av_image_copy_plane(dst_pixels, dst_line_size, src_pixels, src_line_size, bytewidth, min_height);
    }

    return 0;
}

static int android_render_rgb565_on_rgb565(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    return android_render_rgb_on_rgb(out_buffer, overlay, 16);
}

static int android_render_on_rgb565(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_RGBP: {
        return android_render_rgb565_on_rgb565(out_buffer, overlay);
    }
    }

    return -1;
}

static int android_render_rgb32_on_rgb8888(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    return android_render_rgb_on_rgb(out_buffer, overlay, 32);
}

static int android_render_on_rgb8888(ANativeWindow_Buffer *out_buffer, const SDL_VoutOverlay *overlay)
{
    assert(out_buffer);
    assert(overlay);

    switch (overlay->format) {
    case SDL_FCC_RGB4: {
        return android_render_rgb32_on_rgb8888(out_buffer, overlay);
    }
    }

    return -1;
}

int sdl_native_window_display_l(ANativeWindow *native_window, SDL_VoutOverlay *overlay)
{
    int retval;

    if (!native_window) {
        ALOGE("sdl_native_window_display_l: NULL native_window");
        return -1;
    }

    if (!overlay) {
        ALOGE("sdl_native_window_display_l: NULL overlay");
        return -1;
    }

    if (overlay->w <= 0 || overlay->h <= 0) {
        ALOGE("sdl_native_window_display_l: invalid overlay dimensions(%d, %d)", overlay->w, overlay->h);
        return -1;
    }

    int (*fn_cp_image)(ANativeWindow_Buffer *native_buffer, const SDL_VoutOverlay *overlay) = NULL;
    int curr_format = ANativeWindow_getFormat(native_window);
    switch (curr_format) {
    case HAL_PIXEL_FORMAT_YV12:
        fn_cp_image = android_render_on_yv12;
        break;
    case HAL_PIXEL_FORMAT_RGB_565:
        case SDL_FCC_RGBP:
        case SDL_FCC_BGRP:
        fn_cp_image = android_render_on_rgb565;
        break;
    case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case SDL_FCC_RGB4:
        case SDL_FCC_BGR4:
        fn_cp_image = android_render_on_rgb8888;
        break;
    default:
        ALOGE("sdl_native_window_display_l: unexpected buffer format: %d", curr_format);
        break;
    }

    if (!fn_cp_image)
        return -1;

    int buf_w = overlay->w;
    int buf_h = IJKALIGN(overlay->h, 2);

    ANativeWindow_Buffer out_buffer;
    retval = ANativeWindow_lock(native_window, &out_buffer, NULL);
    if (retval < 0) {
        ALOGE("sdl_native_window_display_on_rgb565_l: ANativeWindow_lock: failed %d", retval);
        return retval;
    }

    if (out_buffer.width != buf_w || out_buffer.height != buf_h) {
        ALOGE("unexpected native window buffer (%p)(w:%d, h:%d, fmt:'%.4s'0x%x), expecting (w:%d, h:%d, fmt:'%.4s'0x%x)",
            native_window,
            out_buffer.width, out_buffer.height, (char*)&out_buffer.format, out_buffer.format,
            buf_w, buf_h, (char*)&overlay->format, overlay->format);
        // FIXME: 9 set all black
        ANativeWindow_unlockAndPost(native_window);
        return -1;
    }

    int copy_ret = fn_cp_image(&out_buffer, overlay);
    if (copy_ret < 0) {
        // FIXME: 9 set all black
    }

    retval = ANativeWindow_unlockAndPost(native_window);
    if (retval < 0) {
        ALOGE("sdl_native_window_display_on_rgb565_l: ANativeWindow_unlockAndPost: failed %d", retval);
        return retval;
    }

    return copy_ret;
}
