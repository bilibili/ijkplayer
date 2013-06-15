/*****************************************************************************
 * ijksdl_vout_android.h
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

#ifndef IJKSDL__IJKSDL_VOUT_ANDROID_H
#define IJKSDL__IJKSDL_VOUT_ANDROID_H

#include <jni.h>
#include <android/native_window.h>
#include "ijksdl_stdinc.h"
#include "ijksdl_mutex.h"

typedef struct ANativeWindow ANativeWindow;

SDL_Vout *SDL_VoutAndroid_Create_FromANativeWindow(ANativeWindow *native_window);
SDL_Vout *SDL_VoutAndroid_Create_FromAndroidSurface(jobject android_surface);

void      SDL_VoutAndroid_SetNativeWindow(SDL_Vout vout, ANativeWindow *native_window);
void      SDL_VoutAndroid_SetAndroidSurface(SDL_Vout vout, jobject android_surface);
