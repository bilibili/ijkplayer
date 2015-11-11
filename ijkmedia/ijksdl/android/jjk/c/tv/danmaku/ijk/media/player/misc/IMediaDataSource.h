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

#ifndef JJK__tv_danmaku_ijk_media_player_misc_IMediaDataSource__H
#define JJK__tv_danmaku_ijk_media_player_misc_IMediaDataSource__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jint JJKC_IMediaDataSource__readAt(JNIEnv *env, jobject thiz, jlong position, jbyteArray buffer, jint offset, jint size);
jint JJKC_IMediaDataSource__readAt__catchAll(JNIEnv *env, jobject thiz, jlong position, jbyteArray buffer, jint offset, jint size);
jlong JJKC_IMediaDataSource__getSize(JNIEnv *env, jobject thiz);
jlong JJKC_IMediaDataSource__getSize__catchAll(JNIEnv *env, jobject thiz);
void JJKC_IMediaDataSource__close(JNIEnv *env, jobject thiz);
void JJKC_IMediaDataSource__close__catchAll(JNIEnv *env, jobject thiz);
int JJK_loadClass__JJKC_IMediaDataSource(JNIEnv *env);

#endif//JJK__tv_danmaku_ijk_media_player_misc_IMediaDataSource__H
