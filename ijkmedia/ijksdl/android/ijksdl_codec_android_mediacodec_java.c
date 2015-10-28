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
#include "ijksdl_android_jni.h"
#include "ijksdl_codec_android_mediacodec_internal.h"
#include "ijksdl_codec_android_mediaformat_java.h"
#include "ijksdl_inc_internal_android.h"

static SDL_Class g_amediacodec_class = {
    .name = "AMediaCodecJava",
};

typedef struct SDL_AMediaCodec_Opaque {
    jobject android_media_codec;

    jobjectArray    input_buffer_array;
    jobject         input_buffer;
    jobjectArray    output_buffer_array;
    jobject         output_buffer;
    jobject         output_buffer_info;

    bool            is_input_buffer_valid;
} SDL_AMediaCodec_Opaque;

typedef struct SDL_AMediaCodec_fields_t {
    jclass clazz;

    jmethodID jmid_createByCodecName;
    jmethodID jmid_createDecoderByType;
    jmethodID jmid_configure;
    jmethodID jmid_dequeueInputBuffer;
    jmethodID jmid_dequeueOutputBuffer;
    jmethodID jmid_flush;
    jmethodID jmid_getInputBuffers;
    jmethodID jmid_getOutputBuffers;
    jmethodID jmid_getOutputFormat;
    jmethodID jmid_queueInputBuffer;
    jmethodID jmid_release;
    jmethodID jmid_releaseOutputBuffer;
    jmethodID jmid_reset;
    jmethodID jmid_start;
    jmethodID jmid_stop;

    // >= API-18
    jmethodID jmid_getCodecInfo;
    jmethodID jmid_getName;
} SDL_AMediaCodec_fields_t;
static SDL_AMediaCodec_fields_t g_clazz;

typedef struct SDL_AMediaCodec_BufferInfo_fields_t {
    jclass clazz;

    jmethodID jmid__ctor;

    jfieldID jfid_flags;
    jfieldID jfid_offset;
    jfieldID jfid_presentationTimeUs;
    jfieldID jfid_size;
} SDL_AMediaCodec_BufferInfo_fields_t;
static SDL_AMediaCodec_BufferInfo_fields_t g_clazz_BufferInfo;

int SDL_AMediaCodecJava__loadClass(JNIEnv *env)
{
    jint sdk_int = SDL_Android_GetApiLevel();
    ALOGI("MediaCodec: API-%d\n", sdk_int);
    if (sdk_int < IJK_API_16_JELLY_BEAN) {
        return 0;
    }

    //--------------------
    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "android/media/MediaCodec");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_createByCodecName,  g_clazz.clazz,
        "createByCodecName",  "(Ljava/lang/String;)Landroid/media/MediaCodec;");
    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_createDecoderByType,  g_clazz.clazz,
        "createDecoderByType",  "(Ljava/lang/String;)Landroid/media/MediaCodec;");

    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_configure,            g_clazz.clazz,
        "configure",            "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_dequeueInputBuffer,   g_clazz.clazz,
        "dequeueInputBuffer",   "(J)I");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_dequeueOutputBuffer,  g_clazz.clazz,
        "dequeueOutputBuffer",  "(Landroid/media/MediaCodec$BufferInfo;J)I");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_flush,                g_clazz.clazz,
        "flush",                "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getInputBuffers,      g_clazz.clazz,
        "getInputBuffers",      "()[Ljava/nio/ByteBuffer;");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getOutputBuffers,     g_clazz.clazz,
        "getOutputBuffers",     "()[Ljava/nio/ByteBuffer;");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getOutputFormat,     g_clazz.clazz,
        "getOutputFormat",      "()Landroid/media/MediaFormat;");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_queueInputBuffer,     g_clazz.clazz,
        "queueInputBuffer",     "(IIIJI)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_release,              g_clazz.clazz,
        "release",              "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_releaseOutputBuffer,  g_clazz.clazz,
        "releaseOutputBuffer",  "(IZ)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_start,                g_clazz.clazz,
        "start",                "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_stop,                 g_clazz.clazz,
        "stop",                 "()V");

    /*-
    if (sdk_int >= IJK_API_18_JELLY_BEAN_MR2) {
        IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getCodecInfo,         g_clazz.clazz,
            "getCodecInfo",         "(I)Landroid/media/MediaCodecInfo;");
        IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getName,              g_clazz.clazz,
            "getName",              "()Ljava/lang/String;");
    }
    */


    //--------------------
    IJK_FIND_JAVA_CLASS( env, g_clazz_BufferInfo.clazz, "android/media/MediaCodec$BufferInfo");

    IJK_FIND_JAVA_METHOD(env, g_clazz_BufferInfo.jmid__ctor,                g_clazz_BufferInfo.clazz,
        "<init>"    ,           "()V");

    IJK_FIND_JAVA_FIELD(env, g_clazz_BufferInfo.jfid_flags,                 g_clazz_BufferInfo.clazz,
        "flags",                "I");
    IJK_FIND_JAVA_FIELD(env, g_clazz_BufferInfo.jfid_offset,                g_clazz_BufferInfo.clazz,
        "offset",               "I");
    IJK_FIND_JAVA_FIELD(env, g_clazz_BufferInfo.jfid_presentationTimeUs,    g_clazz_BufferInfo.clazz,
        "presentationTimeUs",   "J");
    IJK_FIND_JAVA_FIELD(env, g_clazz_BufferInfo.jfid_size,                  g_clazz_BufferInfo.clazz,
        "size",                 "I");
    SDLTRACE("android.media.MediaCodec$BufferInfo class loaded");


    SDLTRACE("android.media.MediaCodec class loaded");
    return 0;
}

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
    jobject android_format = (*env)->CallObjectMethod(env, opaque->android_media_codec, g_clazz.jmid_getOutputFormat);
    if (SDL_JNI_CatchException(env) || !android_format) {
        return NULL;
    }

    SDL_AMediaFormat *aformat = SDL_AMediaFormatJava_init(env, android_format);
    SDL_JNI_DeleteLocalRefP(env, &android_format);
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
            (*env)->CallVoidMethod(env, opaque->android_media_codec, g_clazz.jmid_release);
            SDL_JNI_CatchException(env);
        }

        SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_info);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_array);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer_array);
        SDL_JNI_DeleteGlobalRefP(env, &opaque->android_media_codec);
    }

    SDL_AMediaCodec_FreeInternal(acodec);
    return SDL_AMEDIA_OK;
}

inline static int getInputBuffers(JNIEnv *env, SDL_AMediaCodec* acodec)
{
    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec = opaque->android_media_codec;
    SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer_array);
    if (opaque->input_buffer_array)
        return 0;

    jobjectArray local_input_buffer_array = (*env)->CallObjectMethod(env, android_media_codec, g_clazz.jmid_getInputBuffers);
    if (SDL_JNI_CatchException(env) || !local_input_buffer_array) {
        ALOGE("%s: getInputBuffers failed\n", __func__);
        return -1;
    }

    opaque->input_buffer_array = (*env)->NewGlobalRef(env, local_input_buffer_array);
    SDL_JNI_DeleteLocalRefP(env, &local_input_buffer_array);
    if (SDL_JNI_CatchException(env) || !opaque->input_buffer_array) {
        ALOGE("%s: getInputBuffers.NewGlobalRef failed\n", __func__);
        return -1;
    }

    return 0;
}

inline static int getOutputBuffers(JNIEnv *env, SDL_AMediaCodec* acodec)
{
    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_codec = opaque->android_media_codec;
    SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_array);
    if (opaque->output_buffer_array)
        return 0;

    jobjectArray local_output_buffer_array = (*env)->CallObjectMethod(env, android_media_codec, g_clazz.jmid_getOutputBuffers);
    if (SDL_JNI_CatchException(env) || !local_output_buffer_array) {
        ALOGE("%s: getInputBuffers failed\n", __func__);
        return -1;
    }

    opaque->output_buffer_array = (*env)->NewGlobalRef(env, local_output_buffer_array);
    SDL_JNI_DeleteLocalRefP(env, &local_output_buffer_array);
    if (SDL_JNI_CatchException(env) || !opaque->output_buffer_array) {
        ALOGE("%s: getOutputBuffers.NewGlobalRef failed\n", __func__);
        return -1;
    }

    return 0;
}

static sdl_amedia_status_t SDL_AMediaCodecJava_configure_surface(
    JNIEnv*env,
    SDL_AMediaCodec* acodec,
    const SDL_AMediaFormat* aformat,
    jobject android_surface,
    SDL_AMediaCrypto *crypto,
    uint32_t flags)
{
    SDLTRACE("SDL_AMediaCodecJava_configure_surface");

    // TODO: implement SDL_AMediaCrypto
    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    jobject android_media_format = SDL_AMediaFormatJava_getObject(env, aformat);
    jobject android_media_codec  = SDL_AMediaCodecJava_getObject(env, acodec);
    ALOGE("configure acodec:%p format:%p: surface:%p", android_media_codec, android_media_format, android_surface);
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_configure, android_media_format, android_surface, crypto, flags);
    if (SDL_JNI_CatchException(env)) {
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    opaque->is_input_buffer_valid = true;
    SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer_array);
    SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_array);
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
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_start, android_media_codec);
    if (SDL_JNI_CatchException(env)) {
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
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_stop, android_media_codec);
    if (SDL_JNI_CatchException(env)) {
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
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_flush, android_media_codec);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("%s: flush", __func__);
        return SDL_AMEDIA_ERROR_UNKNOWN;
    }

    acodec->object_serial = SDL_AMediaCodec_create_object_serial();
    return SDL_AMEDIA_OK;
}

static uint8_t* SDL_AMediaCodecJava_getInputBuffer(SDL_AMediaCodec* acodec, size_t idx, size_t *out_size)
{
    AMCTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed", __func__);
        return NULL;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    if (0 != getInputBuffers(env, acodec))
        return NULL;

    assert(opaque->input_buffer_array);
    int buffer_count = (*env)->GetArrayLength(env, opaque->input_buffer_array);
    if (SDL_JNI_CatchException(env) || idx < 0 || idx >= buffer_count) {
        ALOGE("%s: idx(%d) < count(%d)\n", __func__, (int)idx, (int)buffer_count);
        return NULL;
    }

    SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer);
    jobject local_input_buffer = (*env)->GetObjectArrayElement(env, opaque->input_buffer_array, idx);
    if (SDL_JNI_CatchException(env) || !local_input_buffer) {
        ALOGE("%s: GetObjectArrayElement failed\n", __func__);
        return NULL;
    }
    opaque->input_buffer = (*env)->NewGlobalRef(env, local_input_buffer);
    SDL_JNI_DeleteLocalRefP(env, &local_input_buffer);
    if (SDL_JNI_CatchException(env) || !opaque->input_buffer) {
        ALOGE("%s: GetObjectArrayElement.NewGlobalRef failed\n", __func__);
        return NULL;
    }

    jlong size = (*env)->GetDirectBufferCapacity(env, opaque->input_buffer);
    void *ptr  = (*env)->GetDirectBufferAddress(env, opaque->input_buffer);

    if (out_size)
        *out_size = size;
    return ptr;
}

static uint8_t* SDL_AMediaCodecJava_getOutputBuffer(SDL_AMediaCodec* acodec, size_t idx, size_t *out_size)
{
    AMCTRACE("%s", __func__);

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_AMediaCodecJava_getOutputBuffer: SetupThreadEnv failed");
        return NULL;
    }

    SDL_AMediaCodec_Opaque *opaque = (SDL_AMediaCodec_Opaque *)acodec->opaque;
    if (0 != getOutputBuffers(env, acodec))
        return NULL;

    assert(opaque->output_buffer_array);

    int buffer_count = (*env)->GetArrayLength(env, opaque->output_buffer_array);
    if (SDL_JNI_CatchException(env) || idx < 0 || idx >= buffer_count) {
        return NULL;
    }

    SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer);
    jobject local_output_buffer = (*env)->GetObjectArrayElement(env, opaque->output_buffer_array, idx);
    if (SDL_JNI_CatchException(env) || !local_output_buffer) {
        return NULL;
    }
    opaque->output_buffer = (*env)->NewGlobalRef(env, local_output_buffer);
    SDL_JNI_DeleteLocalRefP(env, &local_output_buffer);
    if (SDL_JNI_CatchException(env) || !opaque->output_buffer) {
        return NULL;
    }

    jlong size = (*env)->GetDirectBufferCapacity(env, opaque->output_buffer);
    void *ptr  = (*env)->GetDirectBufferAddress(env, opaque->output_buffer);

    if (out_size)
        *out_size = size;
    return ptr;
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
    jint idx = (*env)->CallIntMethod(env, android_media_codec, g_clazz.jmid_dequeueInputBuffer, (jlong)timeoutUs);
    if (SDL_JNI_CatchException(env)) {
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
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_queueInputBuffer, (jint)idx, (jint)offset, (jint)size, (jlong)time, (jint)flags);
    if (SDL_JNI_CatchException(env)) {
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
        opaque->output_buffer_info = SDL_JNI_NewObjectAsGlobalRef(env, g_clazz_BufferInfo.clazz, g_clazz_BufferInfo.jmid__ctor);
        if (SDL_JNI_CatchException(env) || !opaque->output_buffer_info) {
            ALOGE("%s: SDL_JNI_NewObjectAsGlobalRef failed", __func__);
            return AMEDIACODEC__UNKNOWN_ERROR;
        }
    }

    jint idx = AMEDIACODEC__UNKNOWN_ERROR;
    while (1) {
        idx = (*env)->CallIntMethod(env, android_media_codec, g_clazz.jmid_dequeueOutputBuffer, opaque->output_buffer_info, (jlong)timeoutUs);
        if (SDL_JNI_CatchException(env)) {
            ALOGI("%s: Exception\n", __func__);
            return AMEDIACODEC__UNKNOWN_ERROR;
        }
        if (idx == AMEDIACODEC__INFO_OUTPUT_BUFFERS_CHANGED) {
            ALOGI("%s: INFO_OUTPUT_BUFFERS_CHANGED\n", __func__);
            SDL_JNI_DeleteGlobalRefP(env, &opaque->input_buffer_array);
            SDL_JNI_DeleteGlobalRefP(env, &opaque->output_buffer_array);
            continue;
        } else if (idx == AMEDIACODEC__INFO_OUTPUT_FORMAT_CHANGED) {
            ALOGI("%s: INFO_OUTPUT_FORMAT_CHANGED\n", __func__);
        } else if (idx >= 0) {
            AMCTRACE("%s: buffer ready (%d) ====================\n", __func__, idx);
            if (info) {
                info->offset              = (*env)->GetIntField(env, opaque->output_buffer_info, g_clazz_BufferInfo.jfid_offset);
                info->size                = (*env)->GetIntField(env, opaque->output_buffer_info, g_clazz_BufferInfo.jfid_size);
                info->presentationTimeUs  = (*env)->GetLongField(env, opaque->output_buffer_info, g_clazz_BufferInfo.jfid_presentationTimeUs);
                info->flags               = (*env)->GetIntField(env, opaque->output_buffer_info, g_clazz_BufferInfo.jfid_flags);
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
    (*env)->CallVoidMethod(env, android_media_codec, g_clazz.jmid_releaseOutputBuffer, (jint)idx, (jboolean)render);
    if (SDL_JNI_CatchException(env)) {
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
    if (SDL_JNI_CatchException(env) || !global_android_media_codec) {
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

    acodec->func_getInputBuffer         = SDL_AMediaCodecJava_getInputBuffer;
    acodec->func_getOutputBuffer        = SDL_AMediaCodecJava_getOutputBuffer;

    acodec->func_dequeueInputBuffer     = SDL_AMediaCodecJava_dequeueInputBuffer;
    acodec->func_queueInputBuffer       = SDL_AMediaCodecJava_queueInputBuffer;

    acodec->func_dequeueOutputBuffer    = SDL_AMediaCodecJava_dequeueOutputBuffer;
    acodec->func_getOutputFormat        = SDL_AMediaCodecJava_getOutputFormat;
    acodec->func_releaseOutputBuffer    = SDL_AMediaCodecJava_releaseOutputBuffer;

    acodec->func_isInputBuffersValid    = SDL_AMediaCodecJava_isInputBuffersValid;

    SDL_AMediaCodec_increaseReference(acodec);
    return acodec;
}

SDL_AMediaCodec* SDL_AMediaCodecJava_createDecoderByType(JNIEnv *env, const char *mime_type)
{
    SDLTRACE("%s", __func__);

    jstring jmime_type = (*env)->NewStringUTF(env, mime_type);
    if (SDL_JNI_CatchException(env) || !jmime_type) {
        return NULL;
    }

    jobject local_android_media_codec = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_createDecoderByType, jmime_type);
    SDL_JNI_DeleteLocalRefP(env, &jmime_type);
    if (SDL_JNI_CatchException(env) || !local_android_media_codec) {
        return NULL;
    }

    SDL_AMediaCodec* acodec = SDL_AMediaCodecJava_init(env, local_android_media_codec);
    SDL_JNI_DeleteLocalRefP(env, &local_android_media_codec);
    return acodec;
}

SDL_AMediaCodec* SDL_AMediaCodecJava_createByCodecName(JNIEnv *env, const char *codec_name)
{
    SDLTRACE("%s", __func__);

    jstring jcodec_name = (*env)->NewStringUTF(env, codec_name);
    if (SDL_JNI_CatchException(env) || !jcodec_name) {
        return NULL;
    }

    jobject local_android_media_codec = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_createByCodecName, jcodec_name);
    SDL_JNI_DeleteLocalRefP(env, &jcodec_name);
    if (SDL_JNI_CatchException(env) || !local_android_media_codec) {
        return NULL;
    }

    SDL_AMediaCodec* acodec = SDL_AMediaCodecJava_init(env, local_android_media_codec);
    acodec->object_serial = SDL_AMediaCodec_create_object_serial();
    SDL_JNI_DeleteLocalRefP(env, &local_android_media_codec);
    return acodec;
}

