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

#include "IjkMediaPlayer.h"

typedef struct JJKC_IjkMediaPlayer {
    jclass id;

    jfieldID field_mNativeMediaPlayer;
    jfieldID field_mNativeMediaDataSource;
    jmethodID method_postEventFromNative;
    jmethodID method_onSelectCodec;
    jmethodID method_onNativeInvoke;
} JJKC_IjkMediaPlayer;
static JJKC_IjkMediaPlayer class_JJKC_IjkMediaPlayer;

jlong JJKC_IjkMediaPlayer__mNativeMediaPlayer__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetLongField(env, thiz, class_JJKC_IjkMediaPlayer.field_mNativeMediaPlayer);
}

jlong JJKC_IjkMediaPlayer__mNativeMediaPlayer__get__catchAll(JNIEnv *env, jobject thiz)
{
    jlong ret_value = JJKC_IjkMediaPlayer__mNativeMediaPlayer__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_IjkMediaPlayer__mNativeMediaPlayer__set(JNIEnv *env, jobject thiz, jlong value)
{
    (*env)->SetLongField(env, thiz, class_JJKC_IjkMediaPlayer.field_mNativeMediaPlayer, value);
}

void JJKC_IjkMediaPlayer__mNativeMediaPlayer__set__catchAll(JNIEnv *env, jobject thiz, jlong value)
{
    JJKC_IjkMediaPlayer__mNativeMediaPlayer__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

jlong JJKC_IjkMediaPlayer__mNativeMediaDataSource__get(JNIEnv *env, jobject thiz)
{
    return (*env)->GetLongField(env, thiz, class_JJKC_IjkMediaPlayer.field_mNativeMediaDataSource);
}

jlong JJKC_IjkMediaPlayer__mNativeMediaDataSource__get__catchAll(JNIEnv *env, jobject thiz)
{
    jlong ret_value = JJKC_IjkMediaPlayer__mNativeMediaDataSource__get(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_IjkMediaPlayer__mNativeMediaDataSource__set(JNIEnv *env, jobject thiz, jlong value)
{
    (*env)->SetLongField(env, thiz, class_JJKC_IjkMediaPlayer.field_mNativeMediaDataSource, value);
}

void JJKC_IjkMediaPlayer__mNativeMediaDataSource__set__catchAll(JNIEnv *env, jobject thiz, jlong value)
{
    JJKC_IjkMediaPlayer__mNativeMediaDataSource__set(env, thiz, value);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_IjkMediaPlayer__postEventFromNative(JNIEnv *env, jobject weakThiz, jint what, jint arg1, jint arg2, jobject obj)
{
    (*env)->CallStaticVoidMethod(env, class_JJKC_IjkMediaPlayer.id, class_JJKC_IjkMediaPlayer.method_postEventFromNative, weakThiz, what, arg1, arg2, obj);
}

void JJKC_IjkMediaPlayer__postEventFromNative__catchAll(JNIEnv *env, jobject weakThiz, jint what, jint arg1, jint arg2, jobject obj)
{
    JJKC_IjkMediaPlayer__postEventFromNative(env, weakThiz, what, arg1, arg2, obj);
    JJK_ExceptionCheck__catchAll(env);
}

jstring JJKC_IjkMediaPlayer__onSelectCodec(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level)
{
    return (*env)->CallStaticObjectMethod(env, class_JJKC_IjkMediaPlayer.id, class_JJKC_IjkMediaPlayer.method_onSelectCodec, weakThiz, mimeType, profile, level);
}

jstring JJKC_IjkMediaPlayer__onSelectCodec__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level)
{
    jstring ret_object = JJKC_IjkMediaPlayer__onSelectCodec(env, weakThiz, mimeType, profile, level);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jstring JJKC_IjkMediaPlayer__onSelectCodec__asGlobalRef__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level)
{
    jstring ret_object   = NULL;
    jstring local_object = JJKC_IjkMediaPlayer__onSelectCodec__catchAll(env, weakThiz, mimeType, profile, level);
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

const char *JJKC_IjkMediaPlayer__onSelectCodec__asCBuffer(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_IjkMediaPlayer__onSelectCodec(env, weakThiz, mimeType, profile, level);
    if (JJK_ExceptionCheck__throwAny(env) || !local_string) {
        goto fail;
    }

    c_str = (*env)->GetStringUTFChars(env, local_string, NULL );
    if (JJK_ExceptionCheck__throwAny(env) || !c_str) {
        goto fail;
    }

    strlcpy(out_buf, c_str, out_len);
    ret_value = out_buf;

fail:
    JJK_ReleaseStringUTFChars__p(env, local_string, &c_str);
    JJK_DeleteLocalRef__p(env, &local_string);
    return ret_value;
}

const char *JJKC_IjkMediaPlayer__onSelectCodec__asCBuffer__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_IjkMediaPlayer__onSelectCodec__catchAll(env, weakThiz, mimeType, profile, level);
    if (JJK_ExceptionCheck__catchAll(env) || !local_string) {
        goto fail;
    }

    c_str = (*env)->GetStringUTFChars(env, local_string, NULL );
    if (JJK_ExceptionCheck__catchAll(env) || !c_str) {
        goto fail;
    }

    strlcpy(out_buf, c_str, out_len);
    ret_value = out_buf;

fail:
    JJK_ReleaseStringUTFChars__p(env, local_string, &c_str);
    JJK_DeleteLocalRef__p(env, &local_string);
    return ret_value;
}

jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level)
{
    jstring ret_object = NULL;
    jstring mimeType = NULL;

    mimeType = (*env)->NewStringUTF(env, mimeType_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !mimeType)
        goto fail;

    ret_object = JJKC_IjkMediaPlayer__onSelectCodec(env, weakThiz, mimeType, profile, level);
    if (JJK_ExceptionCheck__throwAny(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &mimeType);
    return ret_object;
}

jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level)
{
    jstring ret_object = NULL;
    jstring mimeType = NULL;

    mimeType = (*env)->NewStringUTF(env, mimeType_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !mimeType)
        goto fail;

    ret_object = JJKC_IjkMediaPlayer__onSelectCodec__catchAll(env, weakThiz, mimeType, profile, level);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &mimeType);
    return ret_object;
}

jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString__asGlobalRef__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level)
{
    jstring ret_object   = NULL;
    jstring local_object = JJKC_IjkMediaPlayer__onSelectCodec__withCString__catchAll(env, weakThiz, mimeType_cstr__, profile, level);
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

const char *JJKC_IjkMediaPlayer__onSelectCodec__withCString__asCBuffer(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_IjkMediaPlayer__onSelectCodec__withCString(env, weakThiz, mimeType_cstr__, profile, level);
    if (JJK_ExceptionCheck__throwAny(env) || !local_string) {
        goto fail;
    }

    c_str = (*env)->GetStringUTFChars(env, local_string, NULL );
    if (JJK_ExceptionCheck__throwAny(env) || !c_str) {
        goto fail;
    }

    strlcpy(out_buf, c_str, out_len);
    ret_value = out_buf;

fail:
    JJK_ReleaseStringUTFChars__p(env, local_string, &c_str);
    JJK_DeleteLocalRef__p(env, &local_string);
    return ret_value;
}

const char *JJKC_IjkMediaPlayer__onSelectCodec__withCString__asCBuffer__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_IjkMediaPlayer__onSelectCodec__withCString__catchAll(env, weakThiz, mimeType_cstr__, profile, level);
    if (JJK_ExceptionCheck__catchAll(env) || !local_string) {
        goto fail;
    }

    c_str = (*env)->GetStringUTFChars(env, local_string, NULL );
    if (JJK_ExceptionCheck__catchAll(env) || !c_str) {
        goto fail;
    }

    strlcpy(out_buf, c_str, out_len);
    ret_value = out_buf;

fail:
    JJK_ReleaseStringUTFChars__p(env, local_string, &c_str);
    JJK_DeleteLocalRef__p(env, &local_string);
    return ret_value;
}

jboolean JJKC_IjkMediaPlayer__onNativeInvoke(JNIEnv *env, jobject weakThiz, jint what, jobject args)
{
    return (*env)->CallStaticBooleanMethod(env, class_JJKC_IjkMediaPlayer.id, class_JJKC_IjkMediaPlayer.method_onNativeInvoke, weakThiz, what, args);
}

jboolean JJKC_IjkMediaPlayer__onNativeInvoke__catchAll(JNIEnv *env, jobject weakThiz, jint what, jobject args)
{
    jboolean ret_value = JJKC_IjkMediaPlayer__onNativeInvoke(env, weakThiz, what, args);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return false;
    }

    return ret_value;
}

int JJK_loadClass__JJKC_IjkMediaPlayer(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "tv/danmaku/ijk/media/player/IjkMediaPlayer";
    class_JJKC_IjkMediaPlayer.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_IjkMediaPlayer.id == NULL)
        goto fail;

    class_id = class_JJKC_IjkMediaPlayer.id;
    name     = "mNativeMediaPlayer";
    sign     = "J";
    class_JJKC_IjkMediaPlayer.field_mNativeMediaPlayer = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IjkMediaPlayer.field_mNativeMediaPlayer == NULL)
        goto fail;

    class_id = class_JJKC_IjkMediaPlayer.id;
    name     = "mNativeMediaDataSource";
    sign     = "J";
    class_JJKC_IjkMediaPlayer.field_mNativeMediaDataSource = JJK_GetFieldID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IjkMediaPlayer.field_mNativeMediaDataSource == NULL)
        goto fail;

    class_id = class_JJKC_IjkMediaPlayer.id;
    name     = "postEventFromNative";
    sign     = "(Ljava/lang/Object;IIILjava/lang/Object;)V";
    class_JJKC_IjkMediaPlayer.method_postEventFromNative = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IjkMediaPlayer.method_postEventFromNative == NULL)
        goto fail;

    class_id = class_JJKC_IjkMediaPlayer.id;
    name     = "onSelectCodec";
    sign     = "(Ljava/lang/Object;Ljava/lang/String;II)Ljava/lang/String;";
    class_JJKC_IjkMediaPlayer.method_onSelectCodec = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IjkMediaPlayer.method_onSelectCodec == NULL)
        goto fail;

    class_id = class_JJKC_IjkMediaPlayer.id;
    name     = "onNativeInvoke";
    sign     = "(Ljava/lang/Object;ILandroid/os/Bundle;)Z";
    class_JJKC_IjkMediaPlayer.method_onNativeInvoke = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IjkMediaPlayer.method_onNativeInvoke == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "tv.danmaku.ijk.media.player.IjkMediaPlayer");
    ret = 0;
fail:
    return ret;
}
