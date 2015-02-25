/*****************************************************************************
 * android_bytebuffer.h
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

#ifndef IJKSDL_ANDROID__ANDROID_BYTEBUFFER_H
#define IJKSDL_ANDROID__ANDROID_BYTEBUFFER_H

#include "ijksdl_inc_internal_android.h"

int     ASDK_ByteBuffer__loadClass(JNIEnv *env);
jlong   ASDK_ByteBuffer__getDirectBufferCapacity(JNIEnv *env, jobject byte_buffer);
void   *ASDK_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject byte_buffer);
void    ASDK_ByteBuffer__setDataLimited(JNIEnv *env, jobject byte_buffer, void* data, size_t size);

jobject ASDK_ByteBuffer_allocateDirect(JNIEnv *env, jint capacity);
jobject ASDK_ByteBuffer_allocateDirectAsGlobalRef(JNIEnv *env, jint capacity);
jobject ASDK_ByteBuffer_limit(JNIEnv *env, jobject byte_buffer, jint newLimit);

#endif
