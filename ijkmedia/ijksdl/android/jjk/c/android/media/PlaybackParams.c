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

#include "PlaybackParams.h"

typedef struct JJKC_PlaybackParams {
    jclass id;

    jmethodID method_setSpeed;
} JJKC_PlaybackParams;
static JJKC_PlaybackParams class_JJKC_PlaybackParams;

jobject JJKC_PlaybackParams__setSpeed(JNIEnv *env, jobject thiz, jfloat speed)
{
    return (*env)->CallObjectMethod(env, thiz, class_JJKC_PlaybackParams.method_setSpeed, speed);
}

jobject JJKC_PlaybackParams__setSpeed__catchAll(JNIEnv *env, jobject thiz, jfloat speed)
{
    jobject ret_object = JJKC_PlaybackParams__setSpeed(env, thiz, speed);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_PlaybackParams__setSpeed__asGlobalRef__catchAll(JNIEnv *env, jobject thiz, jfloat speed)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_PlaybackParams__setSpeed__catchAll(env, thiz, speed);
    if (JJK_ExceptionCheck__catchAll(env) || !local_object) {
        ret_object = NULL;
        goto fail;
    }

    ret_object = JJK_NewGlobalRef__catchAll(env, local_object);
    if (!ret_object) {
        ret_object = NULL;
        goto fail;
    }

fail:
    JJK_DeleteLocalRef__p(env, &local_object);
    return ret_object;
}

int JJK_loadClass__JJKC_PlaybackParams(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    api_level = JJK_GetSystemAndroidApiLevel(env);

    if (api_level < 23) {
        ALOGW("JJKLoader: Ignore: '%s' need API %d\n", "android.media.PlaybackParams", api_level);
        goto ignore;
    }

    sign = "android/media/PlaybackParams";
    class_JJKC_PlaybackParams.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_PlaybackParams.id == NULL)
        goto fail;

    class_id = class_JJKC_PlaybackParams.id;
    name     = "setSpeed";
    sign     = "(F)Landroid/media/PlaybackParams;";
    class_JJKC_PlaybackParams.method_setSpeed = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_PlaybackParams.method_setSpeed == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.media.PlaybackParams");
ignore:
    ret = 0;
fail:
    return ret;
}
