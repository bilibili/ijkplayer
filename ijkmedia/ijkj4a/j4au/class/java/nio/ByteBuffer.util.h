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

#ifndef J4A__java_nio_ByteBuffer__UTIL__H
#define J4A__java_nio_ByteBuffer__UTIL__H

#include "j4a/j4a_base.h"
#include "j4a/class/java/nio/ByteBuffer.h"

void *J4AC_java_nio_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject thiz);
void *J4AC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll(JNIEnv *env, jobject thiz);
int   J4AC_java_nio_ByteBuffer__assignData__catchAll(JNIEnv *env, jobject thiz, void* data, size_t size);

#ifdef J4A_HAVE_SIMPLE__J4AC_java_nio_ByteBuffer
#define J4AC_ByteBuffer__getDirectBufferAddress             J4AC_java_nio_ByteBuffer__getDirectBufferAddress
#define J4AC_ByteBuffer__getDirectBufferAddress__catchAll   J4AC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll
#define J4AC_ByteBuffer__assignData__catchAll               J4AC_java_nio_ByteBuffer__assignData__catchAll
#endif

#endif
