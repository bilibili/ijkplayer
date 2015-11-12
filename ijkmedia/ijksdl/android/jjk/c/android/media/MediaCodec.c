/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#include "MediaCodec.h"

typedef struct JJKC_MediaCodec__BufferInfo {
    jclass id;

    jfieldID field_flags;
    jfieldID field_offset;
    jfieldID field_presentationTimeUs;
    jfieldID field_size;
    jmethodID constructor_BufferInfo;
} JJKC_MediaCodec__BufferInfo;
static JJKC_MediaCodec__BufferInfo class_JJKC_MediaCodec__BufferInfo;

typedef struct JJKC_MediaCodec {
    jclass id;

    jmethodID method_createByCodecName;
    jmethodID method_configure;
    jmethodID method_getOutputFormat;
    jmethodID method_getInputBuffers;
    jmethodID method_dequeueInputBuffer;
    jmethodID method_queueInputBuffer;
    jmethodID method_dequeueOutputBuffer;
    jmethodID method_releaseOutputBuffer;
    jmethodID method_start;
    jmethodID method_stop;
    jmethodID method_flush;
    jmethodID method_release;
} JJKC_MediaCodec;
static JJKC_MediaCodec class_JJKC_MediaCodec;

jint JJKC_MediaCodec__BufferInfo__flags__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_flags);
}

jint JJKC_MediaCodec__BufferInfo__flags__get__catchAll(JNIEnv *env, jobject thiz)
{
    jint ret_value = JJKC_MediaCodec__BufferInfo__flags__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__BufferInfo__flags__set(JNIEnv *env, jobject thiz, jint value)
{
    (*env)->SetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_flags, value);
}

void JJKC_MediaCodec__BufferInfo__flags__set__catchAll(JNIEnv *env, jobject thiz, jint value)
{
    JJKC_MediaCodec__BufferInfo__flags__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

jint JJKC_MediaCodec__BufferInfo__offset__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_offset);
}

jint JJKC_MediaCodec__BufferInfo__offset__get__catchAll(JNIEnv *env, jobject thiz)
{
    jint ret_value = JJKC_MediaCodec__BufferInfo__offset__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__BufferInfo__offset__set(JNIEnv *env, jobject thiz, jint value)
{
    (*env)->SetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_offset, value);
}

void JJKC_MediaCodec__BufferInfo__offset__set__catchAll(JNIEnv *env, jobject thiz, jint value)
{
    JJKC_MediaCodec__BufferInfo__offset__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

jlong JJKC_MediaCodec__BufferInfo__presentationTimeUs__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetLongField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_presentationTimeUs);
}

jlong JJKC_MediaCodec__BufferInfo__presentationTimeUs__get__catchAll(JNIEnv *env, jobject thiz)
{
    jlong ret_value = JJKC_MediaCodec__BufferInfo__presentationTimeUs__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__BufferInfo__presentationTimeUs__set(JNIEnv *env, jobject thiz, jlong value)
{
    (*env)->SetLongField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_presentationTimeUs, value);
}

void JJKC_MediaCodec__BufferInfo__presentationTimeUs__set__catchAll(JNIEnv *env, jobject thiz, jlong value)
{
    JJKC_MediaCodec__BufferInfo__presentationTimeUs__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

jint JJKC_MediaCodec__BufferInfo__size__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_size);
}

jint JJKC_MediaCodec__BufferInfo__size__get__catchAll(JNIEnv *env, jobject thiz)
{
    jint ret_value = JJKC_MediaCodec__BufferInfo__size__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__BufferInfo__size__set(JNIEnv *env, jobject thiz, jint value)
{
    (*env)->SetIntField(env, thiz, class_JJKC_MediaCodec__BufferInfo.field_size, value);
}

void JJKC_MediaCodec__BufferInfo__size__set__catchAll(JNIEnv *env, jobject thiz, jint value)
{
    JJKC_MediaCodec__BufferInfo__size__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

jobject JJKC_MediaCodec__BufferInfo__BufferInfo(JNIEnv *env)
{
    return (*env)->NewObject(env, class_JJKC_MediaCodec__BufferInfo.id, class_JJKC_MediaCodec__BufferInfo.constructor_BufferInfo);
}

jobject JJKC_MediaCodec__BufferInfo__BufferInfo__catchAll(JNIEnv *env)
{
    jobject ret_object = JJKC_MediaCodec__BufferInfo__BufferInfo(env);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_MediaCodec__BufferInfo__BufferInfo__asGlobalRef__catchAll(JNIEnv *env)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaCodec__BufferInfo__BufferInfo__catchAll(env);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

int JJK_loadClass__JJKC_MediaCodec__BufferInfo(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "android/media/MediaCodec$BufferInfo";
    class_JJKC_MediaCodec__BufferInfo.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_MediaCodec__BufferInfo.id == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec__BufferInfo.id;
    name     = "flags";
    sign     = "I";
    class_JJKC_MediaCodec__BufferInfo.field_flags = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec__BufferInfo.field_flags == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec__BufferInfo.id;
    name     = "offset";
    sign     = "I";
    class_JJKC_MediaCodec__BufferInfo.field_offset = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec__BufferInfo.field_offset == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec__BufferInfo.id;
    name     = "presentationTimeUs";
    sign     = "J";
    class_JJKC_MediaCodec__BufferInfo.field_presentationTimeUs = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec__BufferInfo.field_presentationTimeUs == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec__BufferInfo.id;
    name     = "size";
    sign     = "I";
    class_JJKC_MediaCodec__BufferInfo.field_size = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec__BufferInfo.field_size == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec__BufferInfo.id;
    name     = "<init>";
    sign     = "()V";
    class_JJKC_MediaCodec__BufferInfo.constructor_BufferInfo = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec__BufferInfo.constructor_BufferInfo == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.media.MediaCodec$BufferInfo");
    ret = 0;
fail:
    return ret;
}

jobject JJKC_MediaCodec__createByCodecName(JNIEnv *env, jstring name)
{
    return (*env)->CallStaticObjectMethod(env, class_JJKC_MediaCodec.id, class_JJKC_MediaCodec.method_createByCodecName, name);
}

jobject JJKC_MediaCodec__createByCodecName__catchAll(JNIEnv *env, jstring name)
{
    jobject ret_object = JJKC_MediaCodec__createByCodecName(env, name);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_MediaCodec__createByCodecName__asGlobalRef__catchAll(JNIEnv *env, jstring name)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaCodec__createByCodecName__catchAll(env, name);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

jobject JJKC_MediaCodec__createByCodecName__withCString(JNIEnv *env, const char *name_cstr__)
{
    jobject ret_object = NULL;
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !name)
        goto fail;

    ret_object = JJKC_MediaCodec__createByCodecName(env, name);
    if (JJK_ExceptionCheck__throwAny(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &name);
    return ret_object;
}

jobject JJKC_MediaCodec__createByCodecName__withCString__catchAll(JNIEnv *env, const char *name_cstr__)
{
    jobject ret_object = NULL;
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !name)
        goto fail;

    ret_object = JJKC_MediaCodec__createByCodecName__catchAll(env, name);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &name);
    return ret_object;
}

jobject JJKC_MediaCodec__createByCodecName__withCString__asGlobalRef__catchAll(JNIEnv *env, const char *name_cstr__)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaCodec__createByCodecName__withCString__catchAll(env, name_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

void JJKC_MediaCodec__configure(JNIEnv *env, jobject thiz, jobject format, jobject surface, jobject crypto, jint flags)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_configure, format, surface, crypto, flags);
}

void JJKC_MediaCodec__configure__catchAll(JNIEnv *env, jobject thiz, jobject format, jobject surface, jobject crypto, jint flags)
{
    JJKC_MediaCodec__configure(env, thiz, format, surface, crypto, flags);
    JJK_ExceptionCheck__catchAll(env);
}

jobject JJKC_MediaCodec__getOutputFormat(JNIEnv *env, jobject thiz)
{
    return (*env)->CallObjectMethod(env, thiz, class_JJKC_MediaCodec.method_getOutputFormat);
}

jobject JJKC_MediaCodec__getOutputFormat__catchAll(JNIEnv *env, jobject thiz)
{
    jobject ret_object = JJKC_MediaCodec__getOutputFormat(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_MediaCodec__getOutputFormat__asGlobalRef__catchAll(JNIEnv *env, jobject thiz)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaCodec__getOutputFormat__catchAll(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

jobjectArray JJKC_MediaCodec__getInputBuffers(JNIEnv *env, jobject thiz)
{
    return (*env)->CallObjectMethod(env, thiz, class_JJKC_MediaCodec.method_getInputBuffers);
}

jobjectArray JJKC_MediaCodec__getInputBuffers__catchAll(JNIEnv *env, jobject thiz)
{
    jobjectArray ret_object = JJKC_MediaCodec__getInputBuffers(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobjectArray JJKC_MediaCodec__getInputBuffers__asGlobalRef__catchAll(JNIEnv *env, jobject thiz)
{
    jobjectArray ret_object   = NULL;
    jobjectArray local_object = JJKC_MediaCodec__getInputBuffers__catchAll(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

jint JJKC_MediaCodec__dequeueInputBuffer(JNIEnv *env, jobject thiz, jlong timeoutUs)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_MediaCodec.method_dequeueInputBuffer, timeoutUs);
}

jint JJKC_MediaCodec__dequeueInputBuffer__catchAll(JNIEnv *env, jobject thiz, jlong timeoutUs)
{
    jint ret_value = JJKC_MediaCodec__dequeueInputBuffer(env, thiz, timeoutUs);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__queueInputBuffer(JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong presentationTimeUs, jint flags)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_queueInputBuffer, index, offset, size, presentationTimeUs, flags);
}

void JJKC_MediaCodec__queueInputBuffer__catchAll(JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong presentationTimeUs, jint flags)
{
    JJKC_MediaCodec__queueInputBuffer(env, thiz, index, offset, size, presentationTimeUs, flags);
    JJK_ExceptionCheck__catchAll(env);
}

jint JJKC_MediaCodec__dequeueOutputBuffer(JNIEnv *env, jobject thiz, jobject info, jlong timeoutUs)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_MediaCodec.method_dequeueOutputBuffer, info, timeoutUs);
}

jint JJKC_MediaCodec__dequeueOutputBuffer__catchAll(JNIEnv *env, jobject thiz, jobject info, jlong timeoutUs)
{
    jint ret_value = JJKC_MediaCodec__dequeueOutputBuffer(env, thiz, info, timeoutUs);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_MediaCodec__releaseOutputBuffer(JNIEnv *env, jobject thiz, jint index, jboolean render)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_releaseOutputBuffer, index, render);
}

void JJKC_MediaCodec__releaseOutputBuffer__catchAll(JNIEnv *env, jobject thiz, jint index, jboolean render)
{
    JJKC_MediaCodec__releaseOutputBuffer(env, thiz, index, render);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaCodec__start(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_start);
}

void JJKC_MediaCodec__start__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_MediaCodec__start(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaCodec__stop(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_stop);
}

void JJKC_MediaCodec__stop__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_MediaCodec__stop(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaCodec__flush(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_flush);
}

void JJKC_MediaCodec__flush__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_MediaCodec__flush(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaCodec__release(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaCodec.method_release);
}

void JJKC_MediaCodec__release__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_MediaCodec__release(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

int JJK_loadClass__JJKC_MediaCodec(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    api_level = JJK_GetSystemAndroidApiLevel(env);

    if (api_level < 16) {
        ALOGW("JJKLoader: Ignore: '%s' need API %d\n", "android.media.MediaCodec", api_level);
        goto ignore;
    }

    sign = "android/media/MediaCodec";
    class_JJKC_MediaCodec.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_MediaCodec.id == NULL)
        goto fail;

    ret = JJK_loadClass__JJKC_MediaCodec__BufferInfo(env);
    if (ret)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "createByCodecName";
    sign     = "(Ljava/lang/String;)Landroid/media/MediaCodec;";
    class_JJKC_MediaCodec.method_createByCodecName = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_createByCodecName == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "configure";
    sign     = "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V";
    class_JJKC_MediaCodec.method_configure = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_configure == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "getOutputFormat";
    sign     = "()Landroid/media/MediaFormat;";
    class_JJKC_MediaCodec.method_getOutputFormat = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_getOutputFormat == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "getInputBuffers";
    sign     = "()[Ljava/nio/ByteBuffer;";
    class_JJKC_MediaCodec.method_getInputBuffers = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_getInputBuffers == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "dequeueInputBuffer";
    sign     = "(J)I";
    class_JJKC_MediaCodec.method_dequeueInputBuffer = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_dequeueInputBuffer == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "queueInputBuffer";
    sign     = "(IIIJI)V";
    class_JJKC_MediaCodec.method_queueInputBuffer = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_queueInputBuffer == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "dequeueOutputBuffer";
    sign     = "(Landroid/media/MediaCodec$BufferInfo;J)I";
    class_JJKC_MediaCodec.method_dequeueOutputBuffer = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_dequeueOutputBuffer == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "releaseOutputBuffer";
    sign     = "(IZ)V";
    class_JJKC_MediaCodec.method_releaseOutputBuffer = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_releaseOutputBuffer == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "start";
    sign     = "()V";
    class_JJKC_MediaCodec.method_start = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_start == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "stop";
    sign     = "()V";
    class_JJKC_MediaCodec.method_stop = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_stop == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "flush";
    sign     = "()V";
    class_JJKC_MediaCodec.method_flush = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_flush == NULL)
        goto fail;

    class_id = class_JJKC_MediaCodec.id;
    name     = "release";
    sign     = "()V";
    class_JJKC_MediaCodec.method_release = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaCodec.method_release == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.media.MediaCodec");
ignore:
    ret = 0;
fail:
    return ret;
}
