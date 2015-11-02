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

#include "ByteBuffer.h"
#include "ByteBuffer.util.h"
#include <string.h>

#ifdef JJK_HAVE__JJKC_ByteBuffer
#define CALL_METHOD(method__) JJKC_ByteBuffer__##method__
#else
#define CALL_METHOD(method__) JJKC_java_nio_ByteBuffer__##method__
#endif

void *JJKC_java_nio_ByteBuffer__getDirectBufferAddress(JNIEnv *env, jobject thiz)
{
    return (*env)->GetDirectBufferAddress(env, thiz);
}

void *JJKC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll(JNIEnv *env, jobject thiz)
{
    void *ret = (*env)->GetDirectBufferAddress(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env) || !ret)
        return NULL;

    return ret;
}

int JJKC_java_nio_ByteBuffer__assignData__catchAll(JNIEnv *env, jobject thiz, void* data, size_t size)
{
    jobject buffer = CALL_METHOD(limit)(env, thiz, size);
    if (JJK_ExceptionCheck__catchAll(env) || !buffer)
        return -1;
    JJK_DeleteLocalRef__p(env, &buffer);

    uint8_t *c_buffer = JJKC_java_nio_ByteBuffer__getDirectBufferAddress__catchAll(env, thiz);
    if (!c_buffer)
        return -1;

    memcpy(c_buffer, data, size);
    return 0;
}
