/*****************************************************************************
 * ijksdl_android.c
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_android_jni.h"

#include "../ijksdl_stdinc.h"

JavaVM *g_jvm;

JavaVM *SDL_AndroidJni_GetJvm()
{
    return g_jvm;
}

jint SDL_AndroidJni_AttachCurrentThread(JNIEnv **p_env, void *thr_args)
{
    JavaVM *jvm = g_jvm;
    if (!jvm) {
        ALOGE("SDL_AndroidJni_GetJvm: AttachCurrentThread: NULL jvm");
        return -1;
    }

    return (*jvm)->AttachCurrentThread(jvm, p_env, thr_args);
}

jint SDL_AndroidJni_DetachCurrentThread()
{
    JavaVM *jvm = g_jvm;
    if (!jvm) {
        ALOGE("SDL_AndroidJni_GetJvm: AttachCurrentThread: NULL jvm");
        return -1;
    }

    return (*jvm)->DetachCurrentThread(jvm);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;

    g_jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
}
