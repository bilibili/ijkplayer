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

#ifndef JJK__java_nio_ByteBuffer__H
#define JJK__java_nio_ByteBuffer__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_ByteBuffer__allocate(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__allocate__catchAll(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__allocate__asGlobalRef__catchAll(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__allocateDirect(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__allocateDirect__catchAll(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__allocateDirect__asGlobalRef__catchAll(JNIEnv *env, jint capacity);
jobject JJKC_ByteBuffer__limit(JNIEnv *env, jobject thiz, jint newLimit);
jobject JJKC_ByteBuffer__limit__catchAll(JNIEnv *env, jobject thiz, jint newLimit);
jobject JJKC_ByteBuffer__limit__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, jint newLimit);
int JJK_loadClass__JJKC_ByteBuffer(JNIEnv *env);

#define JJK_HAVE__JJKC_ByteBuffer

#include "ByteBuffer.util.h"

#endif//JJK__java_nio_ByteBuffer__H
