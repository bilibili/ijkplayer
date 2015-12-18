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

#ifndef JJK__android_media_PlaybackParams__H
#define JJK__android_media_PlaybackParams__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_PlaybackParams__setSpeed(JNIEnv *env, jobject thiz, jfloat speed);
jobject JJKC_PlaybackParams__setSpeed__catchAll(JNIEnv *env, jobject thiz, jfloat speed);
jobject JJKC_PlaybackParams__setSpeed__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, jfloat speed);
int JJK_loadClass__JJKC_PlaybackParams(JNIEnv *env);

#define JJK_HAVE__JJKC_PlaybackParams

#endif//JJK__android_media_PlaybackParams__H
