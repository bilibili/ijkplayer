/*****************************************************************************
 * ijksdl_android_audiotrack.c
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "android_audiotrack.h"

#include <assert.h>
#include "ijkutil/ijkutil.h"

typedef struct SDL_AndroidAudioTrack {
    jobject thiz;

    SDL_AndroidAudioTrack_Spec spec;

    jbyteArray buffer;
    int buffer_capacity;
    int min_buffer_size;
} SDL_AndroidAudioTrack;

typedef struct audio_track_fields_t {
    jclass clazz;

    jmethodID constructor;
    jmethodID getMinBufferSize;
    jmethodID play;
    jmethodID pause;
    jmethodID flush;
    jmethodID stop;
    jmethodID release;
    jmethodID write_byte;
} audio_track_fields_t;
static audio_track_fields_t g_clazz;

#define AT_CHECK_RET(condition__, retval__, ...) \
    if (!(condition__)) { \
        ALOGE(__VA_ARGS__); \
        return (retval__); \
    }

int sdl_audiotrack_global_init(JNIEnv *env)
{
    g_clazz.clazz = (*env)->FindClass(env, "android.media.AudioTrack");
    AT_CHECK_RET(g_clazz.clazz, -1, "missing AudioTrack");

    g_clazz.constructor = (*env)->GetMethodID(env, g_clazz.clazz, "<init>", "(IIIIII)V");
    AT_CHECK_RET(g_clazz.constructor, -1, "missing AudioTrack.<init>");

    g_clazz.getMinBufferSize = (*env)->GetStaticMethodID(env, g_clazz.clazz, "getMinBufferSize", "(III)V");
    AT_CHECK_RET(g_clazz.getMinBufferSize, -1, "missing AudioTrack.getMinBufferSize");

    g_clazz.play = (*env)->GetMethodID(env, g_clazz.clazz, "play", "(V)V");
    AT_CHECK_RET(g_clazz.play, -1, "missing AudioTrack.play");

    g_clazz.pause = (*env)->GetMethodID(env, g_clazz.clazz, "pause", "(V)V");
    AT_CHECK_RET(g_clazz.pause, -1, "missing AudioTrack.pause");

    g_clazz.flush = (*env)->GetMethodID(env, g_clazz.clazz, "flush", "(V)V");
    JNI_CHECK_RET(g_clazz.flush, env, NULL, NULL, -1);

    g_clazz.stop = (*env)->GetMethodID(env, g_clazz.clazz, "stop", "(V)V");
    JNI_CHECK_RET(g_clazz.stop, env, NULL, NULL, -1);

    g_clazz.release = (*env)->GetMethodID(env, g_clazz.clazz, "release", "(V)V");
    JNI_CHECK_RET(g_clazz.release, env, NULL, NULL, -1);

    g_clazz.write_byte = (*env)->GetMethodID(env, g_clazz.clazz, "write", "([BII)V");
    JNI_CHECK_RET(g_clazz.write_byte, env, NULL, NULL, -1);

    return 0;
}

void sdl_audiotrack_get_default_spec(SDL_AndroidAudioTrack_Spec *spec)
{
    assert(spec);
    spec->stream_type = STREAM_MUSIC;
    spec->sample_rate_in_hz = 0;
    spec->channel_config = CHANNEL_OUT_STEREO;
    spec->audio_format = ENCODING_PCM_16BIT;
    spec->buffer_size_in_bytes = 0;
    spec->mode = MODE_STREAM;
}

SDL_AndroidAudioTrack *sdl_audiotrack_new(JNIEnv *env, SDL_AndroidAudioTrack_Spec *spec)
{
    assert(spec);
    SDL_AndroidAudioTrack *atrack = (SDL_AndroidAudioTrack*) malloc(sizeof(SDL_AndroidAudioTrack));
    if (!atrack)
        return NULL;
    memset(atrack, 0, sizeof(SDL_AndroidAudioTrack));

    atrack->spec = *spec;

    jobject thiz = (*env)->NewObject(env, g_clazz.clazz, g_clazz.constructor,
        spec->stream_type, spec->sample_rate_in_hz, spec->channel_config,
        spec->audio_format, spec->buffer_size_in_bytes, spec->mode);
    if (!thiz || (*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_new: NewObject: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        free(atrack);
        return NULL;
    }

    atrack->thiz = (*env)->NewGlobalRef(env, thiz);
    (*env)->DeleteLocalRef(env, thiz);

    atrack->min_buffer_size = sdl_audiotrack_get_min_buffer_size(env, atrack);
    return atrack;
}

int sdl_audiotrack_get_min_buffer_size(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    int retval = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.getMinBufferSize,
        (int) atrack->spec.sample_rate_in_hz,
        (int) atrack->spec.channel_config,
        (int) atrack->spec.audio_format);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_get_min_buffer_size: getMinBufferSize: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

void sdl_audiotrack_play(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, g_clazz.clazz, atrack->thiz, g_clazz.play);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_play: play: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void sdl_audiotrack_pause(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, g_clazz.clazz, atrack->thiz, g_clazz.pause);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_pause: pause: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void sdl_audiotrack_flush(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, g_clazz.clazz, atrack->thiz, g_clazz.flush);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_flush: flush: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void sdl_audiotrack_stop(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, g_clazz.clazz, atrack->thiz, g_clazz.stop);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_stop: stop: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void sdl_audiotrack_release(JNIEnv *env, SDL_AndroidAudioTrack *atrack)
{
    (*env)->CallVoidMethod(env, g_clazz.clazz, atrack->thiz, g_clazz.release);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_release: release: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

int sdl_audiotrack_reserve_buffer(JNIEnv *env, SDL_AndroidAudioTrack *atrack, int len)
{
    if (atrack->buffer && len <= atrack->buffer_capacity)
        return len;

    if (atrack->buffer) {
        (*env)->DeleteLocalRef(env, atrack->buffer);
        atrack->buffer = NULL;
        atrack->buffer_capacity = 0;
    }

    int capacity = IJKMAX(len, atrack->min_buffer_size);
    jbyteArray buffer = (*env)->NewByteArray(env, capacity);
    if (!buffer || (*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_reserve_buffer: NewByteArray: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    atrack->buffer_capacity = capacity;
    atrack->buffer = (*env)->NewGlobalRef(env, buffer);
    (*env)->DeleteLocalRef(env, buffer);
    return capacity;
}

int sdl_audiotrack_write_byte(JNIEnv *env, SDL_AndroidAudioTrack *atrack, uint8_t *data, int len)
{
    if (len <= 0)
        return len;

    int reserved = sdl_audiotrack_reserve_buffer(env, atrack, len);
    if (reserved < len)
        return -1;

    (*env)->SetByteArrayRegion(env, atrack->buffer, 0, len, (jbyte*) data);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_write_byte: SetByteArrayRegion: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    int retval = (*env)->CallIntMethod(env, atrack->thiz, g_clazz.write_byte,
        atrack->buffer, 0, len);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("sdl_audiotrack_write_byte: flush: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}
