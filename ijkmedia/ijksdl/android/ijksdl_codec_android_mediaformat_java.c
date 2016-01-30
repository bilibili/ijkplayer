/*****************************************************************************
 * ijksdl_codec_android_mediaformat_java.c
 *****************************************************************************
 *
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_codec_android_mediaformat_java.h"

#include "j4a/class/android/media/MediaFormat.h"
#include "j4au/class/java/nio/ByteBuffer.util.h"
#include "ijksdl_android_jni.h"
#include "ijksdl_codec_android_mediaformat_internal.h"
#include "ijksdl_inc_internal_android.h"

typedef struct SDL_AMediaFormat_Opaque {
    jobject android_media_format;

    jobject android_byte_buffer;
} SDL_AMediaFormat_Opaque;

jobject SDL_AMediaFormatJava_getObject(JNIEnv *env, const SDL_AMediaFormat *thiz)
{
    if (!thiz || !thiz->opaque)
        return NULL;

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)thiz->opaque;
    return opaque->android_media_format;
}

static jobject getAndroidMediaFormat(JNIEnv *env, const SDL_AMediaFormat* thiz) {
    if (!thiz)
        return NULL;

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)thiz->opaque;
    if (!opaque)
        return NULL;

    return opaque->android_media_format;
}

static sdl_amedia_status_t SDL_AMediaFormatJava_delete(SDL_AMediaFormat* aformat)
{
    if (!aformat)
        return SDL_AMEDIA_OK;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)aformat->opaque;
    if (opaque) {
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_byte_buffer);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_media_format);
    }

    SDL_AMediaFormat_FreeInternal(aformat);
    return SDL_AMEDIA_OK;
}

static bool SDL_AMediaFormatJava_getInt32(SDL_AMediaFormat* aformat, const char* name, int32_t *out)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return false;
    }

    jobject android_media_format = getAndroidMediaFormat(env, aformat);
    if (!android_media_format) {
        ALOGE("%s: getAndroidMediaFormat: failed", __func__);
        return false;
    }

    jint ret = J4AC_MediaFormat__getInteger__withCString(env, android_media_format, name);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: CallIntMethod: failed", __func__);
        return false;
    }

    if (out)
        *out = ret;
    return true;
}

static void SDL_AMediaFormatJava_setInt32(SDL_AMediaFormat* aformat, const char* name, int32_t value)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return;
    }

    jobject android_media_format = getAndroidMediaFormat(env, aformat);
    if (!android_media_format) {
        ALOGE("%s: getAndroidMediaFormat: failed", __func__);
        return;
    }

    J4AC_MediaFormat__setInteger__withCString(env, android_media_format, name, value);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: CallVoidMethod: failed", __func__);
        return;
    }
}

static void SDL_AMediaFormatJava_setBuffer(SDL_AMediaFormat* aformat, const char* name, void* data, size_t size)
{
    int    ret  = -1;
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return;
    }

    SDL_AMediaFormat_Opaque *opaque = (SDL_AMediaFormat_Opaque *)aformat->opaque;
    jobject android_media_format = opaque->android_media_format;
    if (!opaque->android_byte_buffer) {
        opaque->android_byte_buffer = J4AC_ByteBuffer__allocateDirect__asGlobalRef__catchAll(env, size);
        if (!opaque->android_byte_buffer) {
            J4A_FUNC_FAIL_TRACE();
            return;
        }
    }

    ret = J4AC_ByteBuffer__assignData__catchAll(env, opaque->android_byte_buffer, data, size);
    if (ret < 0) {
        J4A_FUNC_FAIL_TRACE();
        return;
    }

    J4AC_MediaFormat__setByteBuffer__withCString(env, android_media_format, name, opaque->android_byte_buffer);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: call jmid_setByteBuffer: failed", __func__);
        return;
    }
}

static void setup_aformat(SDL_AMediaFormat *aformat, jobject android_media_format) {
    SDL_AMediaFormat_Opaque *opaque = aformat->opaque;
    opaque->android_media_format = android_media_format;

    aformat->func_delete    = SDL_AMediaFormatJava_delete;
    aformat->func_getInt32  = SDL_AMediaFormatJava_getInt32;
    aformat->func_setInt32  = SDL_AMediaFormatJava_setInt32;
    aformat->func_setBuffer = SDL_AMediaFormatJava_setBuffer;
}

SDL_AMediaFormat *SDL_AMediaFormatJava_init(JNIEnv *env, jobject android_format)
{
    SDLTRACE("%s", __func__);
    jobject global_android_media_format = (*env)->NewGlobalRef(env, android_format);
    if (J4A_ExceptionCheck__catchAll(env) || !global_android_media_format) {
        return NULL;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormat_CreateInternal(sizeof(SDL_AMediaFormat_Opaque));
    if (!aformat) {
        SDL_JNI_DeleteGlobalRefP(env, &global_android_media_format);
        return NULL;
    }

    setup_aformat(aformat, global_android_media_format);
    return aformat;
}

SDL_AMediaFormat *SDL_AMediaFormatJava_createVideoFormat(JNIEnv *env, const char *mime, int width, int height)
{
    SDLTRACE("%s", __func__);

    jobject android_media_format = J4AC_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll(env, mime, width, height);
    if (J4A_ExceptionCheck__catchAll(env) || !android_media_format) {
        return NULL;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormat_CreateInternal(sizeof(SDL_AMediaFormat_Opaque));
    if (!aformat) {
        SDL_JNI_DeleteGlobalRefP(env, &android_media_format);
        return NULL;
    }

    setup_aformat(aformat, android_media_format);
    SDL_AMediaFormat_setInt32(aformat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, 0);
    return aformat;
}
