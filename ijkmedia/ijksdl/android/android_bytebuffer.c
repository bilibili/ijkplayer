/*****************************************************************************
 * android_bytebuffer.c
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

#include "android_bytebuffer.h"

#include "ijksdl_android_jni.h"

typedef struct ASDK_ByteBuffer_fields_t {
    jclass clazz;

    jmethodID jmid_allocateDirect;
    jmethodID jmid_limit;
} ASDK_ByteBuffer_fields_t;
static ASDK_ByteBuffer_fields_t g_clazz;

int ASDK_ByteBuffer__loadClass(JNIEnv *env)
{
    jint sdk_int = SDL_Android_GetApiLevel();
    if (sdk_int < IJK_API_16_JELLY_BEAN) {
        return 0;
    }

    IJK_FIND_JAVA_CLASS( env, g_clazz.clazz, "java/nio/ByteBuffer");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_allocateDirect,   g_clazz.clazz,
        "allocateDirect",   "(I)Ljava/nio/ByteBuffer;");

    IJK_FIND_JAVA_METHOD(env, g_clazz.jmid_limit,                   g_clazz.clazz,
        "limit",            "(I)Ljava/nio/Buffer;");

    SDLTRACE("java.nio.ByteBuffer class loaded");
    return 0;
}

jlong ASDK_ByteBuffer__getDirectBufferCapacity(JNIEnv *env, jobject byte_buffer)
{
    return (*env)->GetDirectBufferCapacity(env, byte_buffer);
}

void *ASDK_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject byte_buffer)
{
    return (*env)->GetDirectBufferAddress(env, byte_buffer);
}

void ASDK_ByteBuffer__setDataLimited(JNIEnv *env, jobject byte_buffer, void* data, size_t size)
{
    jobject ret_byte_buffer = ASDK_ByteBuffer_limit(env, byte_buffer, size);
    SDL_JNI_DeleteLocalRefP(env, &ret_byte_buffer);
    if (SDL_JNI_RethrowException(env)) {
        return;
    }

    uint8_t *buffer = ASDK_ByteBuffer__getDirectBufferAddress(env, byte_buffer);
    if (SDL_JNI_RethrowException(env) || !buffer) {
        return;
    }

    memcpy(buffer, data, size);
}

jobject ASDK_ByteBuffer_allocateDirect(JNIEnv *env, jint capacity)
{
    SDLTRACE("ASDK_ByteBuffer_allocateDirect");

    jobject byte_buffer = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_allocateDirect, capacity);
    if (SDL_JNI_RethrowException(env) || !byte_buffer) {
        return NULL;
    }

    return byte_buffer;
}

jobject ASDK_ByteBuffer_allocateDirectAsGlobalRef(JNIEnv *env, jint capacity)
{
    jobject local_byte_buffer = ASDK_ByteBuffer_allocateDirect(env, capacity);
    if (SDL_JNI_RethrowException(env) || !local_byte_buffer) {
         return NULL;
    }

    jobject global_byte_buffer = (*env)->NewGlobalRef(env, local_byte_buffer);
    SDL_JNI_DeleteLocalRefP(env, &local_byte_buffer);
    return global_byte_buffer;
}

jobject ASDK_ByteBuffer_limit(JNIEnv *env, jobject byte_buffer, jint newLimit)
{
    SDLTRACE("ASDK_ByteBuffer_limit");

    return (*env)->CallObjectMethod(env, byte_buffer, g_clazz.jmid_limit, newLimit);
}
