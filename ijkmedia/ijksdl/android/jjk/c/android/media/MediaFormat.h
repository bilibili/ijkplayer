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

#ifndef JJK__android_media_MediaFormat__H
#define JJK__android_media_MediaFormat__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_MediaFormat__MediaFormat(JNIEnv *env);
jobject JJKC_MediaFormat__MediaFormat__catchAll(JNIEnv *env);
jobject JJKC_MediaFormat__MediaFormat__asGlobalRef__catchAll(JNIEnv *env);
jobject JJKC_MediaFormat__createVideoFormat(JNIEnv *env, jstring mime, jint width, jint height);
jobject JJKC_MediaFormat__createVideoFormat__catchAll(JNIEnv *env, jstring mime, jint width, jint height);
jobject JJKC_MediaFormat__createVideoFormat__asGlobalRef__catchAll(JNIEnv *env, jstring mime, jint width, jint height);
jobject JJKC_MediaFormat__createVideoFormat__withCString(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jobject JJKC_MediaFormat__createVideoFormat__withCString__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jobject JJKC_MediaFormat__createVideoFormat__withCString__asGlobalRef__catchAll(JNIEnv *env, const char *mime_cstr__, jint width, jint height);
jint JJKC_MediaFormat__getInteger(JNIEnv *env, jobject thiz, jstring name);
jint JJKC_MediaFormat__getInteger__catchAll(JNIEnv *env, jobject thiz, jstring name);
jint JJKC_MediaFormat__getInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__);
jint JJKC_MediaFormat__getInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__);
void JJKC_MediaFormat__setInteger(JNIEnv *env, jobject thiz, jstring name, jint value);
void JJKC_MediaFormat__setInteger__catchAll(JNIEnv *env, jobject thiz, jstring name, jint value);
void JJKC_MediaFormat__setInteger__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value);
void JJKC_MediaFormat__setInteger__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jint value);
void JJKC_MediaFormat__setByteBuffer(JNIEnv *env, jobject thiz, jstring name, jobject bytes);
void JJKC_MediaFormat__setByteBuffer__catchAll(JNIEnv *env, jobject thiz, jstring name, jobject bytes);
void JJKC_MediaFormat__setByteBuffer__withCString(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes);
void JJKC_MediaFormat__setByteBuffer__withCString__catchAll(JNIEnv *env, jobject thiz, const char *name_cstr__, jobject bytes);
int JJK_loadClass__JJKC_MediaFormat(JNIEnv *env);

#endif//JJK__android_media_MediaFormat__H
