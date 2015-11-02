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

#ifndef JJK__java_nio_ByteBuffer__UTIL__H
#define JJK__java_nio_ByteBuffer__UTIL__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"
#include "ByteBuffer.h"

void *JJKC_java_nio_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject thiz);
void *JJKC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll(JNIEnv *env, jobject thiz);
int   JJKC_java_nio_ByteBuffer__assignData__catchAll(JNIEnv *env, jobject thiz, void* data, size_t size);

#ifdef JJK_HAVE__JJKC_ByteBuffer
inline static void *JJKC_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject thiz) {return JJKC_java_nio_ByteBuffer__getDirectBufferAddress(env, thiz);}
inline static void *JJKC_ByteBuffer__getDirectBufferAddress__catchAll(JNIEnv *env, jobject thiz) {return JJKC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll(env, thiz);}
inline static int   JJKC_ByteBuffer__assignData__catchAll(JNIEnv *env, jobject thiz, void* data, size_t size) {return JJKC_java_nio_ByteBuffer__assignData__catchAll(env, thiz, data, size);}
#endif

#endif
