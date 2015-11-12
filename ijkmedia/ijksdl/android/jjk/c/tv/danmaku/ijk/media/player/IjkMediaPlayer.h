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

#ifndef JJK__tv_danmaku_ijk_media_player_IjkMediaPlayer__H
#define JJK__tv_danmaku_ijk_media_player_IjkMediaPlayer__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jlong JJKC_IjkMediaPlayer__mNativeMediaPlayer__get(JNIEnv *env, jobject thiz);
jlong JJKC_IjkMediaPlayer__mNativeMediaPlayer__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_IjkMediaPlayer__mNativeMediaPlayer__set(JNIEnv *env, jobject thiz, jlong value);
void JJKC_IjkMediaPlayer__mNativeMediaPlayer__set__catchAll(JNIEnv *env, jobject thiz, jlong value);
jlong JJKC_IjkMediaPlayer__mNativeMediaDataSource__get(JNIEnv *env, jobject thiz);
jlong JJKC_IjkMediaPlayer__mNativeMediaDataSource__get__catchAll(JNIEnv *env, jobject thiz);
void JJKC_IjkMediaPlayer__mNativeMediaDataSource__set(JNIEnv *env, jobject thiz, jlong value);
void JJKC_IjkMediaPlayer__mNativeMediaDataSource__set__catchAll(JNIEnv *env, jobject thiz, jlong value);
void JJKC_IjkMediaPlayer__postEventFromNative(JNIEnv *env, jobject weakThiz, jint what, jint arg1, jint arg2, jobject obj);
void JJKC_IjkMediaPlayer__postEventFromNative__catchAll(JNIEnv *env, jobject weakThiz, jint what, jint arg1, jint arg2, jobject obj);
jstring JJKC_IjkMediaPlayer__onSelectCodec(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level);
jstring JJKC_IjkMediaPlayer__onSelectCodec__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level);
jstring JJKC_IjkMediaPlayer__onSelectCodec__asGlobalRef__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level);
const char *JJKC_IjkMediaPlayer__onSelectCodec__asCBuffer(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level, char *out_buf, int out_len);
const char *JJKC_IjkMediaPlayer__onSelectCodec__asCBuffer__catchAll(JNIEnv *env, jobject weakThiz, jstring mimeType, jint profile, jint level, char *out_buf, int out_len);
jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level);
jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level);
jstring JJKC_IjkMediaPlayer__onSelectCodec__withCString__asGlobalRef__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level);
const char *JJKC_IjkMediaPlayer__onSelectCodec__withCString__asCBuffer(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level, char *out_buf, int out_len);
const char *JJKC_IjkMediaPlayer__onSelectCodec__withCString__asCBuffer__catchAll(JNIEnv *env, jobject weakThiz, const char *mimeType_cstr__, jint profile, jint level, char *out_buf, int out_len);
jboolean JJKC_IjkMediaPlayer__onNativeInvoke(JNIEnv *env, jobject weakThiz, jint what, jobject args);
jboolean JJKC_IjkMediaPlayer__onNativeInvoke__catchAll(JNIEnv *env, jobject weakThiz, jint what, jobject args);
int JJK_loadClass__JJKC_IjkMediaPlayer(JNIEnv *env);

#endif//JJK__tv_danmaku_ijk_media_player_IjkMediaPlayer__H
