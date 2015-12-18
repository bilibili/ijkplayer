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

#include "AudioTrack.h"
#include "AudioTrack.util.h"
#include "PlaybackParams.h"

#ifdef JJK_HAVE__JJKC_AudioTrack
#define CALL_METHOD(method__) JJKC_AudioTrack__##method__
#else
#define CALL_METHOD(method__) JJKC_android_media_AudioTrack__##method__
#endif

void JJKC_android_media_AudioTrack__setSpeed(JNIEnv *env, jobject thiz, jfloat speed)
{
    if (JJK_GetSystemAndroidApiLevel(env) < 23)
        return;

    jobject temp = NULL;
    jobject params = CALL_METHOD(getPlaybackParams)(env, thiz);
    if (JJK_ExceptionCheck__throwAny(env) || !params)
        goto fail;

#ifdef JJK_HAVE__JJKC_PlaybackParams
    temp = JJKC_PlaybackParams__setSpeed(env, params, speed);
#else
    temp = JJKC_android_media_PlaybackParams__setSpeed(env, params, speed);
#endif
    JJK_DeleteLocalRef__p(env, &temp);
    if (JJK_ExceptionCheck__throwAny(env))
        goto fail;

    ALOGE("%s %f", __func__, (double)speed);
    CALL_METHOD(setPlaybackParams)(env, thiz, params);
    if (JJK_ExceptionCheck__throwAny(env))
        goto fail;

fail:
    JJK_DeleteLocalRef__p(env, &params);
}

void JJKC_android_media_AudioTrack__setSpeed__catchAll(JNIEnv *env, jobject thiz, jfloat speed)
{
    ALOGE("%s", __func__);
    JJKC_android_media_AudioTrack__setSpeed(env, thiz, speed);
    if (JJK_ExceptionCheck__catchAll(env))
        return;

    return;
}
