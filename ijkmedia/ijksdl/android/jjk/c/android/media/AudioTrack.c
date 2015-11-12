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

typedef struct JJKC_AudioTrack {
    jclass id;

    jmethodID constructor_AudioTrack;
    jmethodID method_getMinBufferSize;
    jmethodID method_getMaxVolume;
    jmethodID method_getMinVolume;
    jmethodID method_getNativeOutputSampleRate;
    jmethodID method_play;
    jmethodID method_pause;
    jmethodID method_stop;
    jmethodID method_flush;
    jmethodID method_release;
    jmethodID method_write;
    jmethodID method_setStereoVolume;
    jmethodID method_getAudioSessionId;
} JJKC_AudioTrack;
static JJKC_AudioTrack class_JJKC_AudioTrack;

jobject JJKC_AudioTrack__AudioTrack(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode)
{
    return (*env)->NewObject(env, class_JJKC_AudioTrack.id, class_JJKC_AudioTrack.constructor_AudioTrack, streamType, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);
}

jobject JJKC_AudioTrack__AudioTrack__catchAll(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode)
{
    jobject ret_object = JJKC_AudioTrack__AudioTrack(env, streamType, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);
    if (JJK_ExceptionCheck__catchAll(env) || !ret_object) {
        return NULL;
    }

    return ret_object;
}

jobject JJKC_AudioTrack__AudioTrack__asGlobalRef__catchAll(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode)
{
    jobject ret_object   = NULL;
    jobject local_object = JJKC_AudioTrack__AudioTrack__catchAll(env, streamType, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);
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

jint JJKC_AudioTrack__getMinBufferSize(JNIEnv *env, jint sampleRateInHz, jint channelConfig, jint audioFormat)
{
    return (*env)->CallStaticIntMethod(env, class_JJKC_AudioTrack.id, class_JJKC_AudioTrack.method_getMinBufferSize, sampleRateInHz, channelConfig, audioFormat);
}

jint JJKC_AudioTrack__getMinBufferSize__catchAll(JNIEnv *env, jint sampleRateInHz, jint channelConfig, jint audioFormat)
{
    jint ret_value = JJKC_AudioTrack__getMinBufferSize(env, sampleRateInHz, channelConfig, audioFormat);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jfloat JJKC_AudioTrack__getMaxVolume(JNIEnv *env)
{
    return (*env)->CallStaticFloatMethod(env, class_JJKC_AudioTrack.id, class_JJKC_AudioTrack.method_getMaxVolume);
}

jfloat JJKC_AudioTrack__getMaxVolume__catchAll(JNIEnv *env)
{
    jfloat ret_value = JJKC_AudioTrack__getMaxVolume(env);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jfloat JJKC_AudioTrack__getMinVolume(JNIEnv *env)
{
    return (*env)->CallStaticFloatMethod(env, class_JJKC_AudioTrack.id, class_JJKC_AudioTrack.method_getMinVolume);
}

jfloat JJKC_AudioTrack__getMinVolume__catchAll(JNIEnv *env)
{
    jfloat ret_value = JJKC_AudioTrack__getMinVolume(env);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint JJKC_AudioTrack__getNativeOutputSampleRate(JNIEnv *env, jint streamType)
{
    return (*env)->CallStaticIntMethod(env, class_JJKC_AudioTrack.id, class_JJKC_AudioTrack.method_getNativeOutputSampleRate, streamType);
}

jint JJKC_AudioTrack__getNativeOutputSampleRate__catchAll(JNIEnv *env, jint streamType)
{
    jint ret_value = JJKC_AudioTrack__getNativeOutputSampleRate(env, streamType);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

void JJKC_AudioTrack__play(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_AudioTrack.method_play);
}

void JJKC_AudioTrack__play__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_AudioTrack__play(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_AudioTrack__pause(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_AudioTrack.method_pause);
}

void JJKC_AudioTrack__pause__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_AudioTrack__pause(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_AudioTrack__stop(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_AudioTrack.method_stop);
}

void JJKC_AudioTrack__stop__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_AudioTrack__stop(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_AudioTrack__flush(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_AudioTrack.method_flush);
}

void JJKC_AudioTrack__flush__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_AudioTrack__flush(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

void JJKC_AudioTrack__release(JNIEnv *env, jobject thiz)
{
    (*env)->CallVoidMethod(env, thiz, class_JJKC_AudioTrack.method_release);
}

void JJKC_AudioTrack__release__catchAll(JNIEnv *env, jobject thiz)
{
    JJKC_AudioTrack__release(env, thiz);
    JJK_ExceptionCheck__catchAll(env);
}

jint JJKC_AudioTrack__write(JNIEnv *env, jobject thiz, jbyteArray audioData, jint offsetInBytes, jint sizeInBytes)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_AudioTrack.method_write, audioData, offsetInBytes, sizeInBytes);
}

jint JJKC_AudioTrack__write__catchAll(JNIEnv *env, jobject thiz, jbyteArray audioData, jint offsetInBytes, jint sizeInBytes)
{
    jint ret_value = JJKC_AudioTrack__write(env, thiz, audioData, offsetInBytes, sizeInBytes);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint JJKC_AudioTrack__setStereoVolume(JNIEnv *env, jobject thiz, jfloat leftGain, jfloat rightGain)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_AudioTrack.method_setStereoVolume, leftGain, rightGain);
}

jint JJKC_AudioTrack__setStereoVolume__catchAll(JNIEnv *env, jobject thiz, jfloat leftGain, jfloat rightGain)
{
    jint ret_value = JJKC_AudioTrack__setStereoVolume(env, thiz, leftGain, rightGain);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

jint JJKC_AudioTrack__getAudioSessionId(JNIEnv *env, jobject thiz)
{
    return (*env)->CallIntMethod(env, thiz, class_JJKC_AudioTrack.method_getAudioSessionId);
}

jint JJKC_AudioTrack__getAudioSessionId__catchAll(JNIEnv *env, jobject thiz)
{
    jint ret_value = JJKC_AudioTrack__getAudioSessionId(env, thiz);
    if (JJK_ExceptionCheck__catchAll(env)) {
        return 0;
    }

    return ret_value;
}

int JJK_loadClass__JJKC_AudioTrack(JNIEnv *env)
{
    int         ret                   = -1;
    const char *JJK_UNUSED(name)      = NULL;
    const char *JJK_UNUSED(sign)      = NULL;
    jclass      JJK_UNUSED(class_id)  = NULL;
    int         JJK_UNUSED(api_level) = 0;

    sign = "android/media/AudioTrack";
    class_JJKC_AudioTrack.id = JJK_FindClass__asGlobalRef__catchAll(env, sign);
    if (class_JJKC_AudioTrack.id == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "<init>";
    sign     = "(IIIIII)V";
    class_JJKC_AudioTrack.constructor_AudioTrack = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.constructor_AudioTrack == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "getMinBufferSize";
    sign     = "(III)I";
    class_JJKC_AudioTrack.method_getMinBufferSize = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_getMinBufferSize == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "getMaxVolume";
    sign     = "()F";
    class_JJKC_AudioTrack.method_getMaxVolume = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_getMaxVolume == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "getMinVolume";
    sign     = "()F";
    class_JJKC_AudioTrack.method_getMinVolume = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_getMinVolume == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "getNativeOutputSampleRate";
    sign     = "(I)I";
    class_JJKC_AudioTrack.method_getNativeOutputSampleRate = JJK_GetStaticMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_getNativeOutputSampleRate == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "play";
    sign     = "()V";
    class_JJKC_AudioTrack.method_play = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_play == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "pause";
    sign     = "()V";
    class_JJKC_AudioTrack.method_pause = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_pause == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "stop";
    sign     = "()V";
    class_JJKC_AudioTrack.method_stop = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_stop == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "flush";
    sign     = "()V";
    class_JJKC_AudioTrack.method_flush = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_flush == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "release";
    sign     = "()V";
    class_JJKC_AudioTrack.method_release = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_release == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "write";
    sign     = "([BII)I";
    class_JJKC_AudioTrack.method_write = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_write == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "setStereoVolume";
    sign     = "(FF)I";
    class_JJKC_AudioTrack.method_setStereoVolume = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_setStereoVolume == NULL)
        goto fail;

    class_id = class_JJKC_AudioTrack.id;
    name     = "getAudioSessionId";
    sign     = "()I";
    class_JJKC_AudioTrack.method_getAudioSessionId = JJK_GetMethodID__catchAll(env, class_id, name, sign);
    if (class_JJKC_AudioTrack.method_getAudioSessionId == NULL)
        goto fail;

    ALOGD("JJKLoader: OK: '%s' loaded\n", "android.media.AudioTrack");
    ret = 0;
fail:
    return ret;
}
