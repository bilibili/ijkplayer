/*****************************************************************************
 * ijksdl_codec_android_mediacodec_java.c
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

#include "ijksdl_codec_android_mediacodec_java.h"
#include <assert.h>
#include "j4a/class/android/media/MediaCodec.h"
#include "ijksdl_android_jni.h"
#include "ijksdl_codec_android_mediacodec_internal.h"
#include "ijksdl_codec_android_mediaformat_java.h"
#include "ijksdl_inc_internal_android.h"

static SDL_Class g_amediacodec_class = {
    .name = "AMediaCodecJava",
};

typedef struct SDL_AMediaCodec_Opaque {
    jobject android_media_codec;

    jobject         output_buffer_info;

    bool            is_input_buffer_valid;
} SDL_AMediaCodec_Opaque;

jobject SDL_AMediaCodecJava_getObject(JNIEnv *env, const SDL_AMediaCodec *thiz)
{
    if (!thiz || !thiz->opaque)
        return NULL;

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)thiz->opaque;
    return opaque->android_media_codec;
}

SDL_AMediaFormat *SDL_AMediaCodecJava_getOutputFormat(SDL_AMediaCodec *thiz)
{
    if (!thiz || !thiz->opaque)
        return NULL;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return NULL;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)thiz->opaque;
    jobject local_android_format = J4AC_MediaCodec__getOutputFormat__catchAll(env, opaque->android_media_codec);
    if (!local_android_format) {
        return NULL;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormatJava_init(env, local_android_format);
    SDL_JNI_DeleteLocalRefP(env, &local_android_format);
    return aformat;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_delete(SDL_AMediaCodec* acodec)
{
    ALOGI("%s\n", __func__);
    if (!acodec)
        return SDL_AMEDIA_OK;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaCodecJava_delete: SetupThreadEnv failed");
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    if (opaque) {
        if (opaque->android_media_codec) {
            J4AC_MediaCodec__release__catchAll(env, opaque->android_media_codec);
        }

        SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_info);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_media_codec);
    }

    SDL_AMediaCodec_FreeInternal(acodec);
    return SDL_AMEDIA_OK;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_configure_surface(
    JNIEnv*env,
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    jobject android_surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags)
{
    SDLTRACE("%s", __func__);

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_format = SDL_AMediaFormatJava_getObject(env, aformat);
    jobject android_media_codec  = SDL_AMediaCodecJava_getObject(env, acodec);
    ALOGE("configure acodec:%p format:%p: surface:%p", android_media_codec, android_media_format, android_surface);
    J4AC_MediaCodec__configure(env, android_media_codec, android_media_format, android_surface, crypto, flags);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    opaque->is_input_buffer_valid = true;
    return SDL_AMEDIA_OK;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_start(SDL_AMediaCodec* acodec)
{
    SDLTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec    = opaque->android_media_codec;
    J4AC_MediaCodec__start(env, android_media_codec);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: start failed", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    return SDL_AMEDIA_OK;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_stop(SDL_AMediaCodec* acodec)
{
    SDLTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    jobject android_media_codec = SDL_AMediaCodecJava_getObject(env, acodec);
    J4AC_MediaCodec__stop(env, android_media_codec);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: stop", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    acodec->object_serial = SDL_AMediaCodec_create_object_serial();
    return SDL_AMEDIA_OK;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_flush(SDL_AMediaCodec* acodec)
{
    SDLTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    jobject android_media_codec = SDL_AMediaCodecJava_getObject(env, acodec);
    J4AC_MediaCodec__flush(env, android_media_codec);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: flush", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    acodec->object_serial = SDL_AMediaCodec_create_object_serial();
    return SDL_AMEDIA_OK;
}

static ssize_t SDL_AMediaCodecJava_writeInputData(SDL_AMediaCodec* acodec, size_t idx, const uint8_t *data, size_t size)
{
    AMCTRACE("%s", __func__);
    ssize_t write_ret = -1;
    jobject input_buffer_array = NULL;
    jobject input_buffer = NULL;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return -1;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    input_buffer_array = J4AC_MediaCodec__getInputBuffers__catchAll(env, opaque->android_media_codec);
    if (!input_buffer_array)
        return -1;

    int buffer_count = (*env)->GetArrayLength(env, input_buffer_array);
    if (J4A_ExceptionCheck__catchAll(env) || idx < 0 || idx >= buffer_count) {
        ALOGE("%s: idx(%d) < count(%d)\n", __func__, (int)idx, (int)buffer_count);
        goto fail;
    }

    input_buffer = (*env)->GetObjectArrayElement(env, input_buffer_array, idx);
    if (J4A_ExceptionCheck__catchAll(env) || !input_buffer) {
        ALOGE("%s: GetObjectArrayElement failed\n", __func__);
        goto fail;
    }

    {
        jlong buf_size = (*env)->GetDirectBufferCapacity(env, input_buffer);
        void *buf_ptr  = (*env)->GetDirectBufferAddress(env, input_buffer);

        write_ret = size < buf_size ? size : buf_size;
        memcpy(buf_ptr, data, write_ret);
    }

fail:
    SDL_JNI_DeleteLocalRefP(env, &input_buffer);
    SDL_JNI_DeleteLocalRefP(env, &input_buffer_array);
    return write_ret;
}

ssize_t SDL_AMediaCodecJava_dequeueInputBuffer(SDL_AMediaCodec* acodec, int64_t timeoutUs)
{
    AMCTRACE("%s(%d)", __func__, (int)timeoutUs);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return -1;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    // docs lie, getInputBuffers should be good after
    // m_codec->start() but the internal refs are not
    // setup until much later on some devices.
    //if (-1 == getInputBuffers(env, acodec)) {
    //    ALOGE("%s: getInputBuffers failed", __func__);
    //    return -1;
    //}

    jobject android_media_codec = opaque->android_media_codec;
    jint idx = J4AC_MediaCodec__dequeueInputBuffer(env, android_media_codec, (jlong)timeoutUs);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: dequeueInputBuffer failed", __func__);
        opaque->is_input_buffer_valid = false;
        return -1;
    }

    return idx;
}

sdl_amedia_status_t SDL_AMediaCodecJava_queueInputBuffer(SDL_AMediaCodec* acodec, size_t idx, off_t offset, size_t size, uint64_t time, uint32_t flags)
{
    AMCTRACE("%s: %d", __func__, (int)idx);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaCodecJava_queueInputBuffer: SetupThreadEnv failed");
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec = opaque->android_media_codec;
    J4AC_MediaCodec__queueInputBuffer(env, android_media_codec, (jint)idx, (jint)offset, (jint)size, (jlong)time, (jint)flags);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    return SDL_AMEDIA_OK;
}

ssize_t SDL_AMediaCodecJava_dequeueOutputBuffer(SDL_AMediaCodec* acodec, SDL_AMediaCodecBufferInfo *info, int64_t timeoutUs)
{
    AMCTRACE("%s(%d)", __func__, (int)timeoutUs);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return AMEDIACODEC__UNKNOWN_ERROR;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec = opaque->android_media_codec;
    if (!opaque->output_buffer_info) {
        opaque->output_buffer_info = J4AC_MediaCodec__BufferInfo__BufferInfo__asGlobalRef__catchAll(env);
        if (!opaque->output_buffer_info)
            return AMEDIACODEC__UNKNOWN_ERROR;
    }

    jint idx = AMEDIACODEC__UNKNOWN_ERROR;
    while (1) {
        idx = J4AC_MediaCodec__dequeueOutputBuffer(env, android_media_codec, opaque->output_buffer_info, (jlong)timeoutUs);
        if (J4A_ExceptionCheck__catchAll(env)) {
            ALOGI("%s: Exception\n", __func__);
            return AMEDIACODEC__UNKNOWN_ERROR;
        }
        if (idx == AMEDIACODEC__INFO_OUTPUT_BUFFERS_CHANGED) {
            ALOGI("%s: INFO_OUTPUT_BUFFERS_CHANGED\n", __func__);
            continue;
        } else if (idx == AMEDIACODEC__INFO_OUTPUT_FORMAT_CHANGED) {
            ALOGI("%s: INFO_OUTPUT_FORMAT_CHANGED\n", __func__);
        } else if (idx >= 0) {
            AMCTRACE("%s: buffer ready (%d) ====================\n", __func__, idx);
            if (info) {
                info->offset              = J4AC_MediaCodec__BufferInfo__offset__get__catchAll(env, opaque->output_buffer_info);
                info->size                = J4AC_MediaCodec__BufferInfo__size__get__catchAll(env, opaque->output_buffer_info);
                info->presentationTimeUs  = J4AC_MediaCodec__BufferInfo__presentationTimeUs__get__catchAll(env, opaque->output_buffer_info);
                info->flags               = J4AC_MediaCodec__BufferInfo__flags__get__catchAll(env, opaque->output_buffer_info);
            }
        }
        break;
    }

    return idx;
}

sdl_amedia_status_t SDL_AMediaCodecJava_releaseOutputBuffer(SDL_AMediaCodec* acodec, size_t idx, bool render)
{
    AMCTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s(%d, %s): SetupThreadEnv failed", __func__, (int)idx, render ? "true" : "false");
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec = opaque->android_media_codec;
    J4AC_MediaCodec__releaseOutputBuffer(env, android_media_codec, (jint)idx, (jboolean)render);
    if (J4A_ExceptionCheck__catchAll(env)) {
        ALOGE("%s: releaseOutputBuffer\n", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    return SDL_AMEDIA_OK;
}

bool SDL_AMediaCodecJava_isInputBuffersValid(SDL_AMediaCodec* acodec)
{
    SDL_AMediaCodec_Opaque *opaque = acodec->opaque;
    return opaque->is_input_buffer_valid;
}

static SDL_AMediaCodec* SDL_AMediaCodecJava_init(JNIEnv *env, jobject android_media_codec)
{
    SDLTRACE("%s", __func__);

    jobject global_android_media_codec = (*env)->NewGlobalRef(env, android_media_codec);
    if (J4A_ExceptionCheck__catchAll(env) || !global_android_media_codec) {
        return NULL;
    }

    SDL_AMediaCodec *acodec = SDL_AMediaCodec_CreateInternal(sizeof(SDL_AMediaCodec_Opaque));
    if (!acodec) {
        SDL_JNI_DeleteGlobalRefP(env, &global_android_media_codec);
        return NULL;
    }

    SDL_AMediaCodec_Opaque *opaque = acodec->opaque;
    opaque->android_media_codec         = global_android_media_codec;

    acodec->opaque_class                = &g_amediacodec_class;
    acodec->func_delete                 = SDL_AMediaCodecJava_delete;
    acodec->func_configure              = NULL;
    acodec->func_configure_surface      = SDL_AMediaCodecJava_configure_surface;

    acodec->func_start                  = SDL_AMediaCodecJava_start;
    acodec->func_stop                   = SDL_AMediaCodecJava_stop;
    acodec->func_flush                  = SDL_AMediaCodecJava_flush;

    acodec->func_writeInputData         = SDL_AMediaCodecJava_writeInputData;

    acodec->func_dequeueInputBuffer     = SDL_AMediaCodecJava_dequeueInputBuffer;
    acodec->func_queueInputBuffer       = SDL_AMediaCodecJava_queueInputBuffer;

    acodec->func_dequeueOutputBuffer    = SDL_AMediaCodecJava_dequeueOutputBuffer;
    acodec->func_getOutputFormat        = SDL_AMediaCodecJava_getOutputFormat;
    acodec->func_releaseOutputBuffer    = SDL_AMediaCodecJava_releaseOutputBuffer;

    acodec->func_isInputBuffersValid    = SDL_AMediaCodecJava_isInputBuffersValid;

    SDL_AMediaCodec_increaseReference(acodec);
    return acodec;
}

SDL_AMediaCodec* SDL_AMediaCodecJava_createByCodecName(JNIEnv *env, const char *codec_name)
{
    SDLTRACE("%s", __func__);

    jobject android_media_codec = J4AC_MediaCodec__createByCodecName__withCString__catchAll(env, codec_name);
    if (J4A_ExceptionCheck__catchAll(env) || !android_media_codec) {
        return NULL;
    }

    SDL_AMediaCodec* acodec = SDL_AMediaCodecJava_init(env, android_media_codec);
    acodec->object_serial = SDL_AMediaCodec_create_object_serial();
    SDL_JNI_DeleteLocalRefP(env, &android_media_codec);
    return acodec;
}
