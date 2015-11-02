/*****************************************************************************
 * ijksdl_codec_android_mediaformat_java.h
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIAFORMAT_JAVA_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIAFORMAT_JAVA_H

#include "ijksdl_codec_android_mediaformat.h"

SDL_AMediaFormat *SDL_AMediaFormatJava_init(JNIEnv *env, jobject android_format);
SDL_AMediaFormat *SDL_AMediaFormatJava_createVideoFormat(JNIEnv *env, const char *mime, int width, int height);
jobject           SDL_AMediaFormatJava_getObject(JNIEnv *env, const SDL_AMediaFormat *thiz);

#endif

