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

#include "Bundle.h"

typedef struct JJKC_Bundle {
    jclass id;

    jmethodID constructor_Bundle;
    jmethodID method_getInt;
    jmethodID method_putInt;
    jmethodID method_getString;
    jmethodID method_putString;
    jmethodID method_putParcelableArrayList;
} JJKC_Bundle;
static JJKC_Bundle class_JJKC_Bundle;

jobject JJKC_Bundle__Bundle(JNIEnv *env)
{
    return (*env)->NewObject(env, class_JJKC_Bundle.id, class_JJKC_Bundle.constructor_Bundle);
}

jobject JJKC_Bundle__Bundle__catchAll(JNIEnv *env)
{
    jobject ret_object = JJKC_Bundle__Bundle(env);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_Bundle__Bundle__asGlobalRef__catchAll(JNIEnv *env)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_Bundle__Bundle__catchAll(env);
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

jint JJKC_Bundle__getInt(JNIEnv *env, jobject thiz, jstring key, jint defaultValue)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_Bundle.method_getInt, key, defaultValue);
}

jint JJKC_Bundle__getInt__catchAll(JNIEnv *env, jobject thiz, jstring key, jint defaultValue)
{
    jint ret_value = JJKC_Bundle__getInt(env, thiz, key, defaultValue);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint JJKC_Bundle__getInt__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jint defaultValue)
{
    jint ret_value = 0;
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !key)
        goto fail;

    ret_value = JJKC_Bundle__getInt(env, thiz, key, defaultValue);
    if (JJK_ExceptionCheck__throwAny(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &key);
    return ret_value;
}

jint JJKC_Bundle__getInt__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jint defaultValue)
{
    jint ret_value = 0;
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !key)
        goto fail;

    ret_value = JJKC_Bundle__getInt__catchAll(env, thiz, key, defaultValue);
    if (JJK_ExceptionCheck__catchAll(env)) {
        ret_value = 0;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &key);
    return ret_value;
}

void JJKC_Bundle__putInt(JNIEnv *env, jobject thiz, jstring key, jint value)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_Bundle.method_putInt, key, value);
}

void JJKC_Bundle__putInt__catchAll(JNIEnv *env, jobject thiz, jstring key, jint value)
{
    JJKC_Bundle__putInt(env, thiz, key, value);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_Bundle__putInt__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jint value)
{
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !key)
        goto fail;

    JJKC_Bundle__putInt(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
}

void JJKC_Bundle__putInt__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jint value)
{
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !key)
        goto fail;

    JJKC_Bundle__putInt__catchAll(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
}

jstring JJKC_Bundle__getString(JNIEnv *env, jobject thiz, jstring key)
{
    return (*env)->CallObjectMethod(env, thiz, class_JJKC_Bundle.method_getString, key);
}

jstring JJKC_Bundle__getString__catchAll(JNIEnv *env, jobject thiz, jstring key)
{
    jstring ret_object = JJKC_Bundle__getString(env, thiz, key);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jstring JJKC_Bundle__getString__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, jstring key)
{
    jstring ret_object   = NULL;
    jstring local_object = JJKC_Bundle__getString__catchAll(env, thiz, key);
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

const char *JJKC_Bundle__getString__asCBuffer(JNIEnv *env, jobject thiz, jstring key, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_Bundle__getString(env, thiz, key);
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

const char *JJKC_Bundle__getString__asCBuffer__catchAll(JNIEnv *env, jobject thiz, jstring key, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_Bundle__getString__catchAll(env, thiz, key);
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

jstring JJKC_Bundle__getString__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__)
{
    jstring ret_object = NULL;
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !key)
        goto fail;

    ret_object = JJKC_Bundle__getString(env, thiz, key);
    if (JJK_ExceptionCheck__throwAny(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &key);
    return ret_object;
}

jstring JJKC_Bundle__getString__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__)
{
    jstring ret_object = NULL;
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !key)
        goto fail;

    ret_object = JJKC_Bundle__getString__catchAll(env, thiz, key);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &key);
    return ret_object;
}

jstring JJKC_Bundle__getString__withCString__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__)
{
    jstring ret_object   = NULL;
    jstring local_object = JJKC_Bundle__getString__withCString__catchAll(env, thiz, key_cstr__);
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

const char *JJKC_Bundle__getString__withCString__asCBuffer(JNIEnv *env, jobject thiz, const char *key_cstr__, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_Bundle__getString__withCString(env, thiz, key_cstr__);
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

const char *JJKC_Bundle__getString__withCString__asCBuffer__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, char *out_buf, int out_len)
{
    const char *ret_value = NULL;
    const char *c_str     = NULL;
    jstring local_string = JJKC_Bundle__getString__withCString__catchAll(env, thiz, key_cstr__);
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

void JJKC_Bundle__putString(JNIEnv *env, jobject thiz, jstring key, jstring value)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_Bundle.method_putString, key, value);
}

void JJKC_Bundle__putString__catchAll(JNIEnv *env, jobject thiz, jstring key, jstring value)
{
    JJKC_Bundle__putString(env, thiz, key, value);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_Bundle__putString__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, const char *value_cstr__)
{
    jstring key = NULL;
    jstring value = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !key)
        goto fail;
    value = (*env)->NewStringUTF(env, value_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !value)
        goto fail;

    JJKC_Bundle__putString(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
    JJK_DeleteLocalRef__p(env, &value);
}

void JJKC_Bundle__putString__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, const char *value_cstr__)
{
    jstring key = NULL;
    jstring value = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !key)
        goto fail;
    value = (*env)->NewStringUTF(env, value_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !value)
        goto fail;

    JJKC_Bundle__putString__catchAll(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
    JJK_DeleteLocalRef__p(env, &value);
}

void JJKC_Bundle__putParcelableArrayList(JNIEnv *env, jobject thiz, jstring key, jobject value)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_Bundle.method_putParcelableArrayList, key, value);
}

void JJKC_Bundle__putParcelableArrayList__catchAll(JNIEnv *env, jobject thiz, jstring key, jobject value)
{
    JJKC_Bundle__putParcelableArrayList(env, thiz, key, value);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_Bundle__putParcelableArrayList__withCString(JNIEnv *env, jobject thiz, const char *key_cstr__, jobject value)
{
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__throwAny(env) || !key)
        goto fail;

    JJKC_Bundle__putParcelableArrayList(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
}

void JJKC_Bundle__putParcelableArrayList__withCString__catchAll(JNIEnv *env, jobject thiz, const char *key_cstr__, jobject value)
{
    jstring key = NULL;

    key = (*env)->NewStringUTF(env, key_cstr__);
    if (JJK_ExceptionCheck__catchAll(env) || !key)
        goto fail;

    JJKC_Bundle__putParcelableArrayList__catchAll(env, thiz, key, value);

fail:
    JJK_DeleteLocalRef__p(env, &key);
}

int JJK_loadClass__JJKC_Bundle(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "android/os/Bundle";
    class_JJKC_Bundle.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_Bundle.id == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "<init>";
    sign     = "()V";
    class_JJKC_Bundle.constructor_Bundle = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.constructor_Bundle == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "getInt";
    sign     = "(Ljava/lang/String;I)I";
    class_JJKC_Bundle.method_getInt = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.method_getInt == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "putInt";
    sign     = "(Ljava/lang/String;I)V";
    class_JJKC_Bundle.method_putInt = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.method_putInt == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "getString";
    sign     = "(Ljava/lang/String;)Ljava/lang/String;";
    class_JJKC_Bundle.method_getString = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.method_getString == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "putString";
    sign     = "(Ljava/lang/String;Ljava/lang/String;)V";
    class_JJKC_Bundle.method_putString = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.method_putString == NULL)
        goto fail;

    class_id = class_JJKC_Bundle.id;
    name     = "putParcelableArrayList";
    sign     = "(Ljava/lang/String;Ljava/util/ArrayList;)V";
    class_JJKC_Bundle.method_putParcelableArrayList = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_Bundle.method_putParcelableArrayList == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.os.Bundle");
    ret = 0;
fail:
    return ret;
}
