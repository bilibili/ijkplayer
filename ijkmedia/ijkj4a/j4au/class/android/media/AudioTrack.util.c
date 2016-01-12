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

#include "AudioTrack.util.h"
#include "j4a/class/android/media/PlaybackParams.h"

void J4AC_android_media_AudioTrack__setSpeed(JNIEnv *env, jobject thiz, jfloat speed)
{
    if (J4A_GetSystemAndroidApiLevel(env) < 23)
        return;

    jobject temp = NULL;
    jobject params = J4AC_android_media_AudioTrack__getPlaybackParams(env, thiz);
    if (J4A_ExceptionCheck__throwAny(env) || !params)
        goto fail;

    temp = J4AC_android_media_PlaybackParams__setSpeed(env, params, speed);
    J4A_DeleteLocalRef__p(env, &temp);
    if (J4A_ExceptionCheck__throwAny(env))
        goto fail;

    J4A_ALOGE("%s %f", __func__, (double)speed);
    J4AC_android_media_AudioTrack__setPlaybackParams(env, thiz, params);
    if (J4A_ExceptionCheck__throwAny(env))
        goto fail;

fail:
    J4A_DeleteLocalRef__p(env, &params);
}

void J4AC_android_media_AudioTrack__setSpeed__catchAll(JNIEnv *env, jobject thiz, jfloat speed)
{
    J4A_ALOGE("%s", __func__);
    J4AC_android_media_AudioTrack__setSpeed(env, thiz, speed);
    if (J4A_ExceptionCheck__catchAll(env))
        return;

    return;
}
