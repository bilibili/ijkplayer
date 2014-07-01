/*****************************************************************************
 * ijkadk_android_os_bundle.c
 *****************************************************************************
 *
 * copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ijkadk_android_os_bundle.h"
#include <string.h>
#include <assert.h>

#include "ijkadkinternal.h"

typedef struct ijkadk_android_os_Bundle_class
{
    jclass      clazz;
    jmethodID   constructor;

    jmethodID   putInt;
    jmethodID   getInt;
    jmethodID   putString;
    jmethodID   getString;
} ijkadk_android_os_Bundle_class;
static ijkadk_android_os_Bundle_class g_clazz;


typedef struct ijkBundle{
    jobject thiz;

    char *getString_buffer;
} ijkBundle;


int
ijkadk_android_os_Bundle__loadClass(JNIEnv *env)
{
    IJKADK_FIND_JAVA_CLASS( env, g_clazz.clazz, "android/os/Bundle");

    IJKADK_FIND_JAVA_METHOD(env, g_clazz.constructor, g_clazz.clazz,    "<init>",       "()V");
    IJKADK_FIND_JAVA_METHOD(env, g_clazz.putInt,      g_clazz.clazz,    "putInt",       "(Ljava/lang/String;I)V");
    IJKADK_FIND_JAVA_METHOD(env, g_clazz.getInt,      g_clazz.clazz,    "getInt",       "(Ljava/lang/String;I)I");
    IJKADK_FIND_JAVA_METHOD(env, g_clazz.putString,   g_clazz.clazz,    "putString",    "(Ljava/lang/String;Ljava/lang/String;)V");
    IJKADK_FIND_JAVA_METHOD(env, g_clazz.getString,   g_clazz.clazz,    "getString",    "(Ljava/lang/String;)Ljava/lang/String;");

    return 0;
}


ijkBundle *
ijkBundle_init(JNIEnv *env, jobject java_bundle)
{
    ijkBundle *new_bundle = (ijkBundle *)malloc(sizeof(ijkBundle));
    memset(new_bundle, 0, sizeof(ijkBundle));

    if (java_bundle) {
        new_bundle->thiz = (*env)->NewGlobalRef(env, java_bundle);
    } else {
        java_bundle      = (*env)->NewObject(env, g_clazz.clazz, g_clazz.constructor);
        new_bundle->thiz = (*env)->NewGlobalRef(env, java_bundle);
        (*env)->NewGlobalRef(env, java_bundle);
    }
    return new_bundle;
}

void
ijkBundle_destroyP(JNIEnv *env, ijkBundle **p_bundle)
{
    if (p_bundle == NULL || *p_bundle == NULL)
        return;

    ijkBundle *bundle = *p_bundle;
    if (bundle->thiz) {
        (*env)->DeleteGlobalRef(env, bundle->thiz);
    }

    free(*p_bundle);
    *p_bundle = NULL;
}


void
ijkBundle_putInt(JNIEnv *env, ijkBundle *bundle, const char *key, int value)
{
    assert(key);

    jstring j_key = (*env)->NewStringUTF(env, key);
    (*env)->CallVoidMethod(env, bundle->thiz, g_clazz.putInt, j_key, value);
    (*env)->DeleteLocalRef(env, j_key);
}

int
ijkBundle_getInt(JNIEnv *env, ijkBundle *bundle, const char *key, int default_value)
{
    assert(key);

    jstring j_key = (*env)->NewStringUTF(env, key);
    int value = (*env)->CallIntMethod(env, bundle->thiz, g_clazz.getInt, j_key, default_value);
    (*env)->DeleteLocalRef(env, j_key);

    return value;
}


void
ijkBundle_putString(JNIEnv *env, ijkBundle *bundle, const char *key, const char *value)
{
    assert(key);

    jstring j_key   = (*env)->NewStringUTF(env, key);
    jstring j_value = (*env)->NewStringUTF(env, value);
    (*env)->CallVoidMethod(env, bundle->thiz, g_clazz.putString, j_key, j_value);
    (*env)->DeleteLocalRef(env, j_key);
    (*env)->DeleteLocalRef(env, j_value);
}

const char *
ijkBundle_getString(JNIEnv *env, ijkBundle *bundle, const char *key)
{
    assert(key);

    free(bundle->getString_buffer);
    bundle->getString_buffer = NULL;

    jstring j_key   = (*env)->NewStringUTF(env, key);
    jstring j_value = (*env)->CallObjectMethod(env, bundle->thiz, g_clazz.getString, j_key);

    const char *value = (*env)->GetStringUTFChars(env, j_value, NULL);
    if (value != NULL) {
        bundle->getString_buffer = strdup(value);
    }

    (*env)->DeleteLocalRef(env, j_key);
    (*env)->DeleteLocalRef(env, j_value);

    return bundle->getString_buffer;
}
