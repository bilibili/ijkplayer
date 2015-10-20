/*****************************************************************************
 * android_surface.c
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

#include "android_surface.h"

#include <assert.h>
#include "ijksdl_android_jni.h"

typedef struct ASDK_Surface_fields_t {
    jclass clazz;

    jmethodID jmid_release;
} ASDK_Surface_fields_t;
static ASDK_Surface_fields_t g_clazz;

int ASDK_Surface__loadClass(JNIEnv *env)
{
    jint sdk_int = SDL_Android_GetApiLevel();

    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "android/view/Surface");

    if (sdk_int >= IJK_API_14_ICE_CREAM_SANDWICH) {
        IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_release, g_clazz.clazz,
        "release", "()V");
    }

    return 0;
}

void ASDK_Surface__release(JNIEnv *env, jobject thiz)
{
    jint sdk_int = SDL_Android_GetApiLevel();

    if (sdk_int >= IJK_API_14_ICE_CREAM_SANDWICH) {
        (*env)->CallVoidMethod(env, thiz, g_clazz.jmid_release);
        if (SDL_JNI_RethrowException(env)) {
            goto fail;
        }
    }

fail:
    return;
}

void ASDK_Surface__release__no_throw(JNIEnv *env, jobject thiz)
{
    ASDK_Surface__release(env, thiz);
    if (SDL_JNI_CatchException(env)) {
        ALOGE("%s: failed\n", __func__);
        return;
    }

    return;
}
