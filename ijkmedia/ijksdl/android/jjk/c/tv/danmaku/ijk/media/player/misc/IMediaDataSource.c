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

#include "IMediaDataSource.h"

typedef struct JJKC_IMediaDataSource {
    jclass id;

    jmethodID method_readAt;
    jmethodID method_getSize;
    jmethodID method_close;
} JJKC_IMediaDataSource;
static JJKC_IMediaDataSource class_JJKC_IMediaDataSource;

jint JJKC_IMediaDataSource__readAt(JNIEnv *env, jobject thiz, jlong position, jbyteArray buffer, jint offset, jint size)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_IMediaDataSource.method_readAt, position, buffer, offset, size);
}

jint JJKC_IMediaDataSource__readAt__catchAll(JNIEnv *env, jobject thiz, jlong position, jbyteArray buffer, jint offset, jint size)
{
    jint ret_value = JJKC_IMediaDataSource__readAt(env, thiz, position, buffer, offset, size);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jlong JJKC_IMediaDataSource__getSize(JNIEnv *env, jobject thiz)
{
    return (*env)->CallLongMethod(env, thiz, class_JJKC_IMediaDataSource.method_getSize);
}

jlong JJKC_IMediaDataSource__getSize__catchAll(JNIEnv *env, jobject thiz)
{
    jlong ret_value = JJKC_IMediaDataSource__getSize(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_IMediaDataSource__close(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_IMediaDataSource.method_close);
}

void JJKC_IMediaDataSource__close__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_IMediaDataSource__close(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

int JJK_loadClass__JJKC_IMediaDataSource(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "tv/danmaku/ijk/media/player/misc/IMediaDataSource";
    class_JJKC_IMediaDataSource.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_IMediaDataSource.id == NULL)
        goto fail;

    class_id = class_JJKC_IMediaDataSource.id;
    name     = "readAt";
    sign     = "(J[BII)I";
    class_JJKC_IMediaDataSource.method_readAt = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IMediaDataSource.method_readAt == NULL)
        goto fail;

    class_id = class_JJKC_IMediaDataSource.id;
    name     = "getSize";
    sign     = "()J";
    class_JJKC_IMediaDataSource.method_getSize = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IMediaDataSource.method_getSize == NULL)
        goto fail;

    class_id = class_JJKC_IMediaDataSource.id;
    name     = "close";
    sign     = "()V";
    class_JJKC_IMediaDataSource.method_close = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_IMediaDataSource.method_close == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "tv.danmaku.ijk.media.player.misc.IMediaDataSource");
    ret = 0;
fail:
    return ret;
}
