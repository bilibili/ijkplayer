/*
 * ijksdl_inc_internal_android.h
 *
 * Copyright (c) 2013 Bilibili
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKPLAYER__IJKSDL_INC_INTERNAL_ANDROID_H
#define IJKPLAYER__IJKSDL_INC_INTERNAL_ANDROID_H

#include <stdint.h>
#include <jni.h>

#include "../ijksdl_inc_internal.h"
#include "../ijksdl_misc.h"
#include "../ijksdl_log.h"

enum {
    HAL_PIXEL_FORMAT_RGBA_8888 = 1,
    HAL_PIXEL_FORMAT_RGBX_8888 = 2,
    HAL_PIXEL_FORMAT_RGB_888 = 3,
    HAL_PIXEL_FORMAT_RGB_565 = 4,
    HAL_PIXEL_FORMAT_BGRA_8888 = 5,
    HAL_PIXEL_FORMAT_RGBA_5551 = 6,
    HAL_PIXEL_FORMAT_RGBA_4444 = 7,

    /* 0x8 - 0xFF range unavailable */
    /* 0x100 - 0x1FF HAL implement */
    HAL_PIXEL_FORMAT_YV12 = 0x32315659, // YCrCb 4:2:0 Planar

    HAL_PIXEL_FORMAT_RAW_SENSOR = 0x20,
    HAL_PIXEL_FORMAT_BLOB = 0x21,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 0x22,

    /* Legacy formats (deprecated), used by ImageFormat.java */
    HAL_PIXEL_FORMAT_YCbCr_422_SP = 0x10, // NV16
    HAL_PIXEL_FORMAT_YCrCb_420_SP = 0x11, // NV21
    HAL_PIXEL_FORMAT_YCbCr_422_I = 0x14, // YUY2

};

#endif
