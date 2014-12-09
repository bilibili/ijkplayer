/*
 * ijksdl_inc_internal_android.h
 *
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

#include "../ijksdl_inc_internal.h"
#include "ijkutil/ijkutil.h"

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

#define IJK_FIND_JAVA_CLASS(env__, var__, classsign__) \
    do { \
        jclass clazz = (*env__)->FindClass(env__, classsign__); \
        if (SDL_JNI_CatchException(env) || !(clazz)) { \
            ALOGE("FindClass failed: %s", classsign__); \
            return -1; \
        } \
        var__ = (*env__)->NewGlobalRef(env__, clazz); \
        if (SDL_JNI_CatchException(env) || !(var__)) { \
            ALOGE("FindClass::NewGlobalRef failed: %s", classsign__); \
            (*env__)->DeleteLocalRef(env__, clazz); \
            return -1; \
        } \
        (*env__)->DeleteLocalRef(env__, clazz); \
    } while(0);

#define IJK_FIND_JAVA_METHOD(env__, var__, clazz__, name__, sign__) \
    do { \
        (var__) = (*env__)->GetMethodID((env__), (clazz__), (name__), (sign__)); \
        if (SDL_JNI_CatchException(env) || !(var__)) { \
            ALOGE("GetMethodID failed: %s", name__); \
            return -1; \
        } \
    } while(0);

#define IJK_FIND_JAVA_STATIC_METHOD(env__, var__, clazz__, name__, sign__) \
    do { \
        (var__) = (*env__)->GetStaticMethodID((env__), (clazz__), (name__), (sign__)); \
        if (SDL_JNI_CatchException(env) || !(var__)) { \
            ALOGE("GetMethodID failed: %s", name__); \
            return -1; \
        } \
    } while(0);

#define IJK_FIND_JAVA_FIELD(env__, var__, clazz__, name__, sign__) \
    do { \
        (var__) = (*env__)->GetFieldID((env__), (clazz__), (name__), (sign__)); \
        if (SDL_JNI_CatchException(env) || !(var__)) { \
            ALOGE("GetFieldID failed: %s", name__); \
            return -1; \
        } \
    } while(0);

#endif
