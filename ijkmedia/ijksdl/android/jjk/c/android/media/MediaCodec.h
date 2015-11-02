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

#ifndef JJK__android_media_MediaCodec__H
#define JJK__android_media_MediaCodec__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jint JJKC_MediaCodec__BufferInfo__flags__get(JNIEnv *env, jobject thiz);
jint JJKC_MediaCodec__BufferInfo__flags__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__BufferInfo__flags__set(JNIEnv *env, jobject thiz, jint value);
void JJKC_MediaCodec__BufferInfo__flags__set__catchAll(JNIEnv *env, jobject thiz, jint value);
jint JJKC_MediaCodec__BufferInfo__offset__get(JNIEnv *env, jobject thiz);
jint JJKC_MediaCodec__BufferInfo__offset__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__BufferInfo__offset__set(JNIEnv *env, jobject thiz, jint value);
void JJKC_MediaCodec__BufferInfo__offset__set__catchAll(JNIEnv *env, jobject thiz, jint value);
jlong JJKC_MediaCodec__BufferInfo__presentationTimeUs__get(JNIEnv *env, jobject thiz);
jlong JJKC_MediaCodec__BufferInfo__presentationTimeUs__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__BufferInfo__presentationTimeUs__set(JNIEnv *env, jobject thiz, jlong value);
void JJKC_MediaCodec__BufferInfo__presentationTimeUs__set__catchAll(JNIEnv *env, jobject thiz, jlong value);
jint JJKC_MediaCodec__BufferInfo__size__get(JNIEnv *env, jobject thiz);
jint JJKC_MediaCodec__BufferInfo__size__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__BufferInfo__size__set(JNIEnv *env, jobject thiz, jint value);
void JJKC_MediaCodec__BufferInfo__size__set__catchAll(JNIEnv *env, jobject thiz, jint value);
jobject JJKC_MediaCodec__BufferInfo__BufferInfo(JNIEnv *env);
jobject JJKC_MediaCodec__BufferInfo__BufferInfo__catchAll(JNIEnv *env);
jobject JJKC_MediaCodec__BufferInfo__BufferInfo__asGlobalRef__catchAll(JNIEnv *env);
jobject JJKC_MediaCodec__createByCodecName(JNIEnv *env, jstring name);
jobject JJKC_MediaCodec__createByCodecName__catchAll(JNIEnv *env, jstring name);
jobject JJKC_MediaCodec__createByCodecName__asGlobalRef__catchAll(JNIEnv *env, jstring name);
jobject JJKC_MediaCodec__createByCodecName__withCString(JNIEnv *env, const char *name_cstr__);
jobject JJKC_MediaCodec__createByCodecName__withCString__catchAll(JNIEnv *env, const char *name_cstr__);
jobject JJKC_MediaCodec__createByCodecName__withCString__asGlobalRef__catchAll(JNIEnv *env, const char *name_cstr__);
void JJKC_MediaCodec__configure(JNIEnv *env, jobject thiz, jobject format, jobject surface, jobject crypto, jint flags);
void JJKC_MediaCodec__configure__catchAll(JNIEnv *env, jobject thiz, jobject format, jobject surface, jobject crypto, jint flags);
jobject JJKC_MediaCodec__getOutputFormat(JNIEnv *env, jobject thiz);
jobject JJKC_MediaCodec__getOutputFormat__catchAll(JNIEnv *env, jobject thiz);
jobject JJKC_MediaCodec__getOutputFormat__asGlobalRef__catchAll(JNIEnv *env, jobject thiz);
jobjectArray JJKC_MediaCodec__getInputBuffers(JNIEnv *env, jobject thiz);
jobjectArray JJKC_MediaCodec__getInputBuffers__catchAll(JNIEnv *env, jobject thiz);
jobjectArray JJKC_MediaCodec__getInputBuffers__asGlobalRef__catchAll(JNIEnv *env, jobject thiz);
jint JJKC_MediaCodec__dequeueInputBuffer(JNIEnv *env, jobject thiz, jlong timeoutUs);
jint JJKC_MediaCodec__dequeueInputBuffer__catchAll(JNIEnv *env, jobject thiz, jlong timeoutUs);
void JJKC_MediaCodec__queueInputBuffer(JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong presentationTimeUs, jint flags);
void JJKC_MediaCodec__queueInputBuffer__catchAll(JNIEnv *env, jobject thiz, jint index, jint offset, jint size, jlong presentationTimeUs, jint flags);
jint JJKC_MediaCodec__dequeueOutputBuffer(JNIEnv *env, jobject thiz, jobject info, jlong timeoutUs);
jint JJKC_MediaCodec__dequeueOutputBuffer__catchAll(JNIEnv *env, jobject thiz, jobject info, jlong timeoutUs);
void JJKC_MediaCodec__releaseOutputBuffer(JNIEnv *env, jobject thiz, jint index, jboolean render);
void JJKC_MediaCodec__releaseOutputBuffer__catchAll(JNIEnv *env, jobject thiz, jint index, jboolean render);
void JJKC_MediaCodec__start(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__start__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__stop(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__stop__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__flush(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__flush__catchAll(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__release(JNIEnv *env, jobject thiz);
void JJKC_MediaCodec__release__catchAll(JNIEnv *env, jobject thiz);
int JJK_loadClass__JJKC_MediaCodec(JNIEnv *env);

#endif//JJK__android_media_MediaCodec__H
