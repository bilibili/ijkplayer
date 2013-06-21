/*****************************************************************************
 * ijksdl_vout_android_surface.c
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

#include "ijksdl_vout_android_surface.h"

#include <android/native_window_jni.h>
#include "ijksdl_vout_android_nativewindow.h"

SDL_Vout *SDL_VoutAndroid_CreateForAndroidSurface()
{
    return SDL_VoutAndroid_CreateForANativeWindow();
}

static void SDL_VoutAndroid_SetAndroidSurface_n(JNIEnv *env, SDL_Vout *vout, jobject android_surface)
{
    if (!android_surface)
        return;

    ANativeWindow *native_window = ANativeWindow_fromSurface(env, android_surface);
    if (!native_window)
        return;

    SDL_VoutAndroid_SetNativeWindow(vout, native_window);
    ANativeWindow_release(native_window);
}

void SDL_VoutAndroid_SetAndroidSurface(SDL_Vout *vout, jobject android_surface)
{
    if (!android_surface)
        return;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_AndroidJni_AttachCurrentThread(&env, NULL)) {
        ALOGE("SDL_VoutAndroid_SetAndroidSurface: AttachCurrentThread: failed");
        return;
    }

    SDL_VoutAndroid_SetAndroidSurface_n(env, vout, android_surface);

    SDL_AndroidJni_DetachCurrentThread();
}
