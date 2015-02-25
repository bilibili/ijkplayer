/*****************************************************************************
 * android_build.c
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

#include "android_build.h"

#include "ijksdl_android_jni.h"

typedef struct ASDK_Build_VERSION_fields_t {
    jclass clazz;

    jfieldID jfid_SDK_INT;
} ASDK_Build_VERSION_fields_t;
static ASDK_Build_VERSION_fields_t g_clazz_VERSION;

int ASDK_Build__loadClass(JNIEnv *env)
{
    IJK_FIND_JAVA_CLASS( env, g_clazz_VERSION.clazz, "android/os/Build$VERSION");

    IJK_FIND_JAVA_STATIC_FIELD(env, g_clazz_VERSION.jfid_SDK_INT,   g_clazz_VERSION.clazz,
        "SDK_INT",   "I");

    return 0;
}

int ASDK_Build_VERSION__SDK_INT(JNIEnv *env)
{
    jint sdk_int = (*env)->GetStaticIntField(env, g_clazz_VERSION.clazz, g_clazz_VERSION.jfid_SDK_INT);
    if (SDL_JNI_RethrowException(env)) {
        return 0;
    }

    return sdk_int;
}
