/*****************************************************************************
 * android_bundle.c
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

#include "android_bundle.h"

#include <assert.h>
#include "ijksdl_android_jni.h"

typedef struct ASDK_Bundle_fields_t {
    jclass clazz;

    jmethodID jmid_init;
    jmethodID jmid_getInt;
    jmethodID jmid_putInt;
    jmethodID jmid_putParcelableArrayList;
    jmethodID jmid_getString;
    jmethodID jmid_putString;
} ASDK_Bundle_fields_t;
static ASDK_Bundle_fields_t g_clazz;

int ASDK_Bundle__loadClass(JNIEnv *env)
{
    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "android/os/Bundle");

    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_init,                      g_clazz.clazz,
        "<init>",                   "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getInt,                    g_clazz.clazz,
        "getInt",                   "(Ljava/lang/String;I)I");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_putInt,                    g_clazz.clazz,
        "putInt",                   "(Ljava/lang/String;I)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_putParcelableArrayList,    g_clazz.clazz,
        "putParcelableArrayList",   "(Ljava/lang/String;Ljava/util/ArrayList;)V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_getString,                 g_clazz.clazz,
        "getString",                "(Ljava/lang/String;)Ljava/lang/String;");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_putString,                 g_clazz.clazz,
        "putString",                "(Ljava/lang/String;Ljava/lang/String;)V");

    return 0;
}

jobject ASDK_Bundle__init(JNIEnv *env)
{
    jobject local_ref = (*env)->NewObject(env, g_clazz.clazz, g_clazz.jmid_init);
    if (SDL_JNI_RethrowException(env) || !local_ref) {
        return NULL;
    }

    return local_ref;
}

int ASDK_Bundle__getInt_c(JNIEnv *env, jobject thiz, const char *key, int defaultValue)
{
    assert(key);
    jstring jkey = NULL;
    int     value = defaultValue;

    jkey = (*env)->NewStringUTF(env, key);
    if (SDL_JNI_RethrowException(env) || !jkey) {
        goto fail;
    }

    value = (*env)->CallIntMethod(env, thiz, g_clazz.jmid_getInt, jkey, defaultValue);
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

fail:
    SDL_JNI_DeleteLocalRefP(env, &jkey);
    return value;
}

void ASDK_Bundle__putInt_c(JNIEnv *env, jobject thiz, const char *key, int value)
{
    assert(key);
    jstring jkey = NULL;

    jkey = (*env)->NewStringUTF(env, key);
    if (SDL_JNI_RethrowException(env) || !jkey) {
        goto fail;
    }

    (*env)->CallVoidMethod(env, thiz, g_clazz.jmid_putInt, jkey, value);
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

fail:
    SDL_JNI_DeleteLocalRefP(env, &jkey);
    return;
}

void ASDK_Bundle__putParcelableArrayList_c(JNIEnv *env, jobject thiz, const char *key, jobject value)
{
    assert(key);
    jstring jkey = NULL;

    jkey = (*env)->NewStringUTF(env, key);
    if (SDL_JNI_RethrowException(env) || !jkey) {
        goto fail;
    }

    (*env)->CallVoidMethod(env, thiz, g_clazz.jmid_putParcelableArrayList, jkey, value);
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

fail:
    SDL_JNI_DeleteLocalRefP(env, &jkey);
    return;
}

void ASDK_Bundle__getString_cbuf(JNIEnv *env, jobject thiz, const char *key, char *value, size_t size)
{
    assert(key);
    jstring jkey   = NULL;
    jstring jvalue = NULL;
    const char *c_value = NULL;

    assert(value);
    value[0] = 0;

    jkey = (*env)->NewStringUTF(env, key);
    if (SDL_JNI_RethrowException(env) || !jkey) {
        goto fail;
    }

    jvalue = (*env)->CallObjectMethod(env, thiz, g_clazz.jmid_getString, jkey);
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

    c_value = (*env)->GetStringUTFChars(env, jvalue, NULL );
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

    strlcpy(value, c_value, size);

fail:
    if (c_value) {
        (*env)->ReleaseStringUTFChars(env, jvalue, c_value);
        c_value = NULL;
    }
    SDL_JNI_DeleteLocalRefP(env, &jvalue);
    SDL_JNI_DeleteLocalRefP(env, &jkey);
}

void ASDK_Bundle__putString_c(JNIEnv *env, jobject thiz, const char *key, const char *value)
{
    assert(key);
    jstring jkey = NULL;
    jstring jvalue = NULL;

    jkey = (*env)->NewStringUTF(env, key);
    if (SDL_JNI_RethrowException(env) || !jkey) {
        goto fail;
    }

    if (value) {
        jvalue = (*env)->NewStringUTF(env, value);
        if (SDL_JNI_RethrowException(env) || !jvalue) {
            goto fail;
        }
    }

    (*env)->CallVoidMethod(env, thiz, g_clazz.jmid_putString, jkey, jvalue);
    if (SDL_JNI_RethrowException(env)) {
        goto fail;
    }

fail:
    SDL_JNI_DeleteLocalRefP(env, &jkey);
    SDL_JNI_DeleteLocalRefP(env, &jvalue);
    return;
}
