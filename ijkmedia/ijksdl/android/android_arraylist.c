/*****************************************************************************
 * android_arraylist.c
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

#include "android_arraylist.h"

#include "ijksdl_android_jni.h"

typedef struct ASDK_ArrayList_fields_t {
    jclass clazz;

    jmethodID jmid_init;
    jmethodID jmid_add;
} ASDK_ArrayList_fields_t;
static ASDK_ArrayList_fields_t g_clazz;

int ASDK_ArrayList__loadClass(JNIEnv *env)
{
    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "java/util/ArrayList");

    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_init,    g_clazz.clazz,
        "<init>",                   "()V");
    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_add,     g_clazz.clazz,
        "add",                      "(Ljava/lang/Object;)Z");

    return 0;
}

jobject ASDK_ArrayList__init(JNIEnv *env)
{
    jobject local_ref = (*env)->NewObject(env, g_clazz.clazz, g_clazz.jmid_init);
    if (SDL_JNI_RethrowException(env) || !local_ref) {
        return NULL;
    }

    return local_ref;
}

jboolean ASDK_ArrayList__add(JNIEnv *env, jobject thiz, jobject elem)
{
    jboolean ret = (*env)->CallBooleanMethod(env, thiz, g_clazz.jmid_add, elem);
    if (SDL_JNI_RethrowException(env)) {
        ret = JNI_FALSE;
        goto fail;
    }

    ret = JNI_TRUE;
fail:
    return ret;
}

