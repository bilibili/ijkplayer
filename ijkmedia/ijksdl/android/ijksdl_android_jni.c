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

#include "ijkadk/ijkadkutils.h"

#include "ijksdl_inc_internal_android.h"
#include "android_audiotrack.h"

static JavaVM *g_jvm;

JavaVM *SDL_AndroidJni_GetJvm()
{
    return g_jvm;
}

jint SDL_AndroidJni_SetupThreadEnv(JNIEnv **p_env)
{
    return ijkadk_setup_thread_env(p_env);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    int retval;
    JNIEnv* env = NULL;

    g_jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    retval = sdl_audiotrack_global_init(env);
    JNI_CHECK_RET(retval == 0, env, NULL, NULL, -1);

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
}
