/*
 * ffmpeg_api_jni.c
 *
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ffmpeg_api_jni.h"

#include <assert.h>
#include <string.h>
#include <jni.h>
#include "../ff_ffinc.h"
#include "ijksdl/ijksdl_log.h"
#include "ijksdl/android/ijksdl_android_jni.h"

#define JNI_CLASS_FFMPEG_API "tv/danmaku/ijk/media/player/ffmpeg/FFmpegApi"

typedef struct ffmpeg_api_fields_t {
    jclass clazz;
} ffmpeg_api_fields_t;
static ffmpeg_api_fields_t g_clazz;

static jstring
FFmpegApi_av_base64_encode(JNIEnv *env, jclass clazz, jbyteArray in)
{
    jstring ret_string = NULL;
    char*   out_buffer = 0;
    int     out_size   = 0;
    jbyte*  in_buffer  = 0;
    jsize   in_size    = (*env)->GetArrayLength(env, in);
    if (in_size <= 0)
        goto fail;

    in_buffer = (*env)->GetByteArrayElements(env, in, NULL);
    if (!in_buffer)
        goto fail;

    out_size = AV_BASE64_SIZE(in_size);
    out_buffer = malloc(out_size + 1);
    if (!out_buffer)
        goto fail;
    out_buffer[out_size] = 0;

    if (!av_base64_encode(out_buffer, out_size, (const uint8_t *)in_buffer, in_size))
        goto fail;

    ret_string = (*env)->NewStringUTF(env, out_buffer);
fail:
    if (in_buffer) {
        (*env)->ReleaseByteArrayElements(env, in, in_buffer, JNI_ABORT);
        in_buffer = NULL;
    }
    if (out_buffer) {
        free(out_buffer);
        out_buffer = NULL;
    }
    return ret_string;
}

static JNINativeMethod g_methods[] = {
    {"av_base64_encode", "([B)Ljava/lang/String;", (void *) FFmpegApi_av_base64_encode},
};

int FFmpegApi_global_init(JNIEnv *env)
{
    int ret = 0;

    IJK_FIND_JAVA_CLASS(env, g_clazz.clazz, JNI_CLASS_FFMPEG_API);
    (*env)->RegisterNatives(env, g_clazz.clazz, g_methods, NELEM(g_methods));

    return ret;
}
