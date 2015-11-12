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

#include "MediaFormat.h"

typedef struct JJKC_MediaFormat {
    jclass id;

    jmethodID constructor_MediaFormat;
    jmethodID method_createVideoFormat;
    jmethodID method_getInteger;
    jmethodID method_setInteger;
    jmethodID method_setByteBuffer;
} JJKC_MediaFormat;
static JJKC_MediaFormat class_JJKC_MediaFormat;

jobject JJKC_MediaFormat__MediaFormat(JNIEnv *env)
{
    return (*env)->NewObject(env, class_JJKC_MediaFormat.id, class_JJKC_MediaFormat.constructor_MediaFormat);
}

jobject JJKC_MediaFormat__MediaFormat__catchAll(JNIEnv *env)
{
    jobject ret_object = JJKC_MediaFormat__MediaFormat(env);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_MediaFormat__MediaFormat__asGlobalRef__catchAll(JNIEnv *env)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaFormat__MediaFormat__catchAll(env);
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

jobject JJKC_MediaFormat__createVideoFormat(JNIEnv *env, jstring mime, jint width, jint height)
{
    return (*env)->CallStaticObjectMethod(env, class_JJKC_MediaFormat.id, class_JJKC_MediaFormat.method_createVideoFormat, mime, width, height);
}

jobject JJKC_MediaFormat__createVideoFormat__catchAll(JNIEnv *env, jstring mime, jint width, jint height)
{
    jobject ret_object = JJKC_MediaFormat__createVideoFormat(env, mime, width, height);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_MediaFormat__createVideoFormat__asGlobalRef__catchAll(JNIEnv *env, jstring mime, jint width, jint height)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaFormat__createVideoFormat__catchAll(env, mime, width, height);
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

jobject JJKC_MediaFormat__createVideoFormat__withCString(JNIEnv *env, const char *mime_cstr__, jint width, jint height)
{
    jobject ret_object = NULL;
    jstring mime = NULL;

    mime = (*env)->NewStringUTF(env, mime_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !mime)
        goto fail;

    ret_object = JJKC_MediaFormat__createVideoFormat(env, mime, width, height);
    if (JJK_ExceptionCheck__throwAny(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &mime);
    return ret_object;
}

jobject JJKC_MediaFormat__createVideoFormat__withCString__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height)
{
    jobject ret_object = NULL;
    jstring mime = NULL;

    mime = (*env)->NewStringUTF(env, mime_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !mime)
        goto fail;

    ret_object = JJKC_MediaFormat__createVideoFormat__catchAll(env, mime, width, height);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &mime);
    return ret_object;
}

jobject JJKC_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_MediaFormat__createVideoFormat__withCString__catchAll(env, mime_cstr__, width, height);
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

jint JJKC_MediaFormat__getInteger(JNIEnv *env, jobject thiz, jstring name)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_MediaFormat.method_getInteger, name);
}

jint JJKC_MediaFormat__getInteger__catchAll(JNIEnv *env, jobject thiz, jstring name)
{
    jint ret_value = JJKC_MediaFormat__getInteger(env, thiz, name);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint JJKC_MediaFormat__getInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__)
{
    jint ret_value = 0;
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !name)
        goto fail;

    ret_value = JJKC_MediaFormat__getInteger(env, thiz, name);
    if (JJK_ExceptionCheck__throwAny(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &name);
    return ret_value;
}

jint JJKC_MediaFormat__getInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__)
{
    jint ret_value = 0;
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !name)
        goto fail;

    ret_value = JJKC_MediaFormat__getInteger__catchAll(env, thiz, name);
    if (JJK_ExceptionCheck__catchAll(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &name);
    return ret_value;
}

void JJKC_MediaFormat__setInteger(JNIEnv *env, jobject thiz, jstring name, jint value)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaFormat.method_setInteger, name, value);
}

void JJKC_MediaFormat__setInteger__catchAll(JNIEnv *env, jobject thiz, jstring name, jint value)
{
    JJKC_MediaFormat__setInteger(env, thiz, name, value);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaFormat__setInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value)
{
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !name)
        goto fail;

    JJKC_MediaFormat__setInteger(env, thiz, name, value);

fail:
    JJK_DeleteLocalRef__p(env, &name);
}

void JJKC_MediaFormat__setInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value)
{
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !name)
        goto fail;

    JJKC_MediaFormat__setInteger__catchAll(env, thiz, name, value);

fail:
    JJK_DeleteLocalRef__p(env, &name);
}

void JJKC_MediaFormat__setByteBuffer(JNIEnv *env, jobject thiz, jstring name, jobject bytes)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_MediaFormat.method_setByteBuffer, name, bytes);
}

void JJKC_MediaFormat__setByteBuffer__catchAll(JNIEnv *env, jobject thiz, jstring name, jobject bytes)
{
    JJKC_MediaFormat__setByteBuffer(env, thiz, name, bytes);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_MediaFormat__setByteBuffer__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes)
{
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !name)
        goto fail;

    JJKC_MediaFormat__setByteBuffer(env, thiz, name, bytes);

fail:
    JJK_DeleteLocalRef__p(env, &name);
}

void JJKC_MediaFormat__setByteBuffer__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes)
{
    jstring name = NULL;

    name = (*env)->NewStringUTF(env, name_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !name)
        goto fail;

    JJKC_MediaFormat__setByteBuffer__catchAll(env, thiz, name, bytes);

fail:
    JJK_DeleteLocalRef__p(env, &name);
}

int JJK_loadClass__JJKC_MediaFormat(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    api_level = JJK_GetSystemAndroidApiLevel(env);

    if (api_level < 16) {
        ALOGW("JJKLoader: Ignore: '%s' need API %d\n", "android.media.MediaFormat", api_level);
        goto ignore;
    }

    sign = "android/media/MediaFormat";
    class_JJKC_MediaFormat.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_MediaFormat.id == NULL)
        goto fail;

    class_id = class_JJKC_MediaFormat.id;
    name     = "<init>";
    sign     = "()V";
    class_JJKC_MediaFormat.constructor_MediaFormat = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaFormat.constructor_MediaFormat == NULL)
        goto fail;

    class_id = class_JJKC_MediaFormat.id;
    name     = "createVideoFormat";
    sign     = "(Ljava/lang/String;II)Landroid/media/MediaFormat;";
    class_JJKC_MediaFormat.method_createVideoFormat = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaFormat.method_createVideoFormat == NULL)
        goto fail;

    class_id = class_JJKC_MediaFormat.id;
    name     = "getInteger";
    sign     = "(Ljava/lang/String;)I";
    class_JJKC_MediaFormat.method_getInteger = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaFormat.method_getInteger == NULL)
        goto fail;

    class_id = class_JJKC_MediaFormat.id;
    name     = "setInteger";
    sign     = "(Ljava/lang/String;I)V";
    class_JJKC_MediaFormat.method_setInteger = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaFormat.method_setInteger == NULL)
        goto fail;

    class_id = class_JJKC_MediaFormat.id;
    name     = "setByteBuffer";
    sign     = "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V";
    class_JJKC_MediaFormat.method_setByteBuffer = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_MediaFormat.method_setByteBuffer == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.media.MediaFormat");
ignore:
    ret = 0;
fail:
    return ret;
}
