/*****************************************************************************
 * android_audiotrack.c
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
#include "ijksdl_android_jni.h"
#include "../ijksdl_inc_internal.h"
#include "../ijksdl_audio.h"

#ifdef SDLTRACE
#undef SDLTRACE
#define SDLTRACE(...)
#endif

typedef struct AudioChannelMapEntry {
    Uint8 sdl_channel;
    int android_channel;
    const char *sdl_name;
    const char *android_name;
} AudioChannelMapEntry;
static AudioChannelMapEntry g_audio_channel_map[] = {
    { 2, CHANNEL_OUT_STEREO, "2-chan", "CHANNEL_OUT_STEREO" },
    { 1, CHANNEL_OUT_MONO, "1-chan", "CHANNEL_OUT_MONO" },
};

typedef struct AudioFormatMapEntry {
    SDL_AudioFormat sdl_format;
    int android_format;
    const char *sdl_name;
    const char *android_name;
} AudioFormatMapEntry;
static AudioFormatMapEntry g_audio_format_map[] = {
    { AUDIO_S16SYS, ENCODING_PCM_16BIT, "AUDIO_S16SYS", "ENCODING_PCM_16BIT" },
    { AUDIO_U8,     ENCODING_PCM_8BIT,  "AUDIO_U8",     "ENCODING_PCM_8BIT" },
    { AUDIO_F32,    ENCODING_PCM_FLOAT, "AUDIO_F32",    "ENCODING_PCM_FLOAT" },
};

static Uint8 find_sdl_channel(int android_channel)
{
    for (int i = 0; i < NELEM(g_audio_channel_map); ++i) {
        AudioChannelMapEntry *entry = &g_audio_channel_map[i];
        if (entry->android_channel == android_channel)
            return entry->sdl_channel;
    }
    return 0;
}

static int find_android_channel(int sdl_channel)
{
    for (int i = 0; i < NELEM(g_audio_channel_map); ++i) {
        AudioChannelMapEntry *entry = &g_audio_channel_map[i];
        if (entry->sdl_channel == sdl_channel)
            return entry->android_channel;
    }
    return CHANNEL_OUT_INVALID;
}

static SDL_AudioFormat find_sdl_format(int android_format)
{
    for (int i = 0; i < NELEM(g_audio_format_map); ++i) {
        AudioFormatMapEntry *entry = &g_audio_format_map[i];
        if (entry->android_format == android_format)
            return entry->sdl_format;
    }
    return AUDIO_INVALID;
}

static int find_android_format(int sdl_format)
{
    for (int i = 0; i < NELEM(g_audio_format_map); ++i) {
        AudioFormatMapEntry *entry = &g_audio_format_map[i];
        if (entry->sdl_format == sdl_format)
            return entry->android_format;
    }
    return ENCODING_INVALID;
}

typedef struct SDL_Android_AudioTrack {
    jobject thiz;

    SDL_Android_AudioTrack_Spec spec;

    jbyteArray  byte_buffer;
    int         byte_buffer_capacity;
    int         min_buffer_size;
    float       max_volume;
    float       min_volume;

    jfloatArray float_buffer;
    int         float_buffer_capacity;
} SDL_Android_AudioTrack;

typedef struct AudioTrack_fields_t {
    jclass clazz;

    jmethodID constructor;
    jmethodID getMinBufferSize;
    jmethodID getMaxVolume;
    jmethodID getMinVolume;
    jmethodID getNativeOutputSampleRate;

    jmethodID play;
    jmethodID pause;
    jmethodID flush;
    jmethodID stop;
    jmethodID release;
    jmethodID write_byte;
    jmethodID setStereoVolume;

    jmethodID write_float;
} AudioTrack_fields_t;
static AudioTrack_fields_t g_clazz;

int SDL_Android_AudioTrack_global_init(JNIEnv *env)
{
    jclass clazz;
    jint sdk_int = SDL_Android_GetApiLevel();

    clazz = (*env)->FindClass(env, "android/media/AudioTrack");
    IJK_CHECK_RET(clazz, -1, "missing AudioTrack");

    // FindClass returns LocalReference
    g_clazz.clazz = (*env)->NewGlobalRef(env, clazz);
    IJK_CHECK_RET(g_clazz.clazz, -1, "AudioTrack NewGlobalRef failed");
    (*env)->DeleteLocalRef(env, clazz);

    g_clazz.constructor = (*env)->GetMethodID(env, g_clazz.clazz, "<init>", "(IIIIII)V");
    IJK_CHECK_RET(g_clazz.constructor, -1, "missing AudioTrack.<init>");

    g_clazz.getMinBufferSize = (*env)->GetStaticMethodID(env, g_clazz.clazz, "getMinBufferSize", "(III)I");
    IJK_CHECK_RET(g_clazz.getMinBufferSize, -1, "missing AudioTrack.getMinBufferSize");

    g_clazz.getMaxVolume = (*env)->GetStaticMethodID(env, g_clazz.clazz, "getMaxVolume", "()F");
    IJK_CHECK_RET(g_clazz.getMaxVolume, -1, "missing AudioTrack.getMaxVolume");

    g_clazz.getMinVolume = (*env)->GetStaticMethodID(env, g_clazz.clazz, "getMinVolume", "()F");
    IJK_CHECK_RET(g_clazz.getMinVolume, -1, "missing AudioTrack.getMinVolume");

    g_clazz.getNativeOutputSampleRate = (*env)->GetStaticMethodID(env, g_clazz.clazz, "getNativeOutputSampleRate", "(I)I");
    IJK_CHECK_RET(g_clazz.getNativeOutputSampleRate, -1, "missing AudioTrack.getNativeOutputSampleRate");

    g_clazz.play = (*env)->GetMethodID(env, g_clazz.clazz, "play", "()V");
    IJK_CHECK_RET(g_clazz.play, -1, "missing AudioTrack.play");

    g_clazz.pause = (*env)->GetMethodID(env, g_clazz.clazz, "pause", "()V");
    IJK_CHECK_RET(g_clazz.pause, -1, "missing AudioTrack.pause");

    g_clazz.flush = (*env)->GetMethodID(env, g_clazz.clazz, "flush", "()V");
    IJK_CHECK_RET(g_clazz.flush, -1, "missing AudioTrack.flush");

    g_clazz.stop = (*env)->GetMethodID(env, g_clazz.clazz, "stop", "()V");
    IJK_CHECK_RET(g_clazz.stop, -1, "missing AudioTrack.stop");

    g_clazz.release = (*env)->GetMethodID(env, g_clazz.clazz, "release", "()V");
    IJK_CHECK_RET(g_clazz.release, -1, "missing AudioTrack.release");

    g_clazz.write_byte = (*env)->GetMethodID(env, g_clazz.clazz, "write", "([BII)I");
    IJK_CHECK_RET(g_clazz.write_byte, -1, "missing AudioTrack.write(byte[], ...)");

    g_clazz.setStereoVolume = (*env)->GetMethodID(env, g_clazz.clazz, "setStereoVolume", "(FF)I");
    IJK_CHECK_RET(g_clazz.setStereoVolume, -1, "missing AudioTrack.setStereoVolume");

    if (sdk_int >= IJK_API_21_LOLLIPOP) {
        g_clazz.write_float = (*env)->GetMethodID(env, g_clazz.clazz, "write", "([FIII)I");
        IJK_CHECK_RET(g_clazz.write_float, -1, "missing AudioTrack.write(float[], ...)");
    }

    SDLTRACE("android.media.AudioTrack class loaded");
    return 0;
}

static void SDL_Android_AudioTrack_get_default_spec(SDL_Android_AudioTrack_Spec *spec)
{
    assert(spec);
    spec->stream_type = STREAM_MUSIC;
    spec->sample_rate_in_hz = 0;
    spec->channel_config = CHANNEL_OUT_STEREO;
    spec->audio_format = ENCODING_PCM_16BIT;
    spec->buffer_size_in_bytes = 0;
    spec->mode = MODE_STREAM;
}

static int audiotrack_get_min_buffer_size(JNIEnv *env, SDL_Android_AudioTrack_Spec *spec)
{
    SDLTRACE("audiotrack_get_min_buffer_size");
    int retval = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.getMinBufferSize,
        (int) spec->sample_rate_in_hz,
        (int) spec->channel_config,
        (int) spec->audio_format);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_min_buffer_size: getMinBufferSize: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

static float audiotrack_get_max_volume(JNIEnv *env)
{
    SDLTRACE("audiotrack_get_max_volume");
    float retval = (*env)->CallStaticFloatMethod(env, g_clazz.clazz, g_clazz.getMaxVolume);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_max_volume: getMaxVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

static float audiotrack_get_min_volume(JNIEnv *env)
{
    SDLTRACE("audiotrack_get_min_volume");
    float retval = (*env)->CallStaticFloatMethod(env, g_clazz.clazz, g_clazz.getMinVolume);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_min_volume: getMinVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

#define STREAM_MUSIC 3
int audiotrack_get_native_output_sample_rate(JNIEnv *env)
{
    if (!env) {
        if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
            ALOGE("%s: SetupThreadEnv failed", __func__);
            return -1;
        }
    }

    SDLTRACE("audiotrack_get_native_output_sample_rate");
    int retval = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.getNativeOutputSampleRate, STREAM_MUSIC);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_get_native_output_sample_rate: getMinVolume: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return -1;
    }

    return retval;
}

static int audiotrack_set_stereo_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left, float right)
{
    int retval = (*env)->CallIntMethod(env, atrack->thiz, g_clazz.setStereoVolume, left, right);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("audiotrack_set_stereo_volume: write_byte: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}

void SDL_Android_AudioTrack_set_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left_volume, float right_volume)
{
    audiotrack_set_stereo_volume(env, atrack, left_volume, right_volume);
}

SDL_Android_AudioTrack *SDL_Android_AudioTrack_new_from_spec(JNIEnv *env, SDL_Android_AudioTrack_Spec *spec)
{
    assert(spec);
    jint sdk_int = SDL_Android_GetApiLevel();

    switch (spec->channel_config) {
    case CHANNEL_OUT_MONO:
        ALOGI("SDL_Android_AudioTrack: %s", "CHANNEL_OUT_MONO");
        break;
    case CHANNEL_OUT_STEREO:
        ALOGI("SDL_Android_AudioTrack: %s", "CHANNEL_OUT_STEREO");
        break;
    default:
        ALOGE("SDL_Android_AudioTrack_new_from_spec: invalid channel %d", spec->channel_config);
        return NULL;
    }

    switch (spec->audio_format) {
    case ENCODING_PCM_16BIT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_16BIT");
        break;
    case ENCODING_PCM_8BIT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_8BIT");
        break;
    case ENCODING_PCM_FLOAT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_FLOAT");
        if (sdk_int < IJK_API_21_LOLLIPOP) {
            ALOGI("SDL_Android_AudioTrack: %s need API 21 or above", "ENCODING_PCM_FLOAT");
            return NULL;
        }
        break;
    default:
        ALOGE("SDL_Android_AudioTrack_new_from_spec: invalid format %d", spec->audio_format);
        return NULL;
    }

    SDL_Android_AudioTrack *atrack = (SDL_Android_AudioTrack*) mallocz(sizeof(SDL_Android_AudioTrack));
    if (!atrack) {
        (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.release);
        return NULL;
    }
    atrack->spec = *spec;

    if (atrack->spec.sample_rate_in_hz < 4000 || atrack->spec.sample_rate_in_hz > 48000) {
        int native_sample_rate_in_hz = audiotrack_get_native_output_sample_rate(env);
        if (native_sample_rate_in_hz > 0) {
            ALOGE("SDL_Android_AudioTrack_new: cast sample rate %d to %d:",
                atrack->spec.sample_rate_in_hz,
                native_sample_rate_in_hz);
            atrack->spec.sample_rate_in_hz = native_sample_rate_in_hz;
        }
    }

    int min_buffer_size = audiotrack_get_min_buffer_size(env, &atrack->spec);
    if (min_buffer_size <= 0) {
        ALOGE("SDL_Android_AudioTrack_new: SDL_Android_AudioTrack_get_min_buffer_size: return %d:", min_buffer_size);
        free(atrack);
        return NULL;
    }

    jobject thiz = (*env)->NewObject(env, g_clazz.clazz, g_clazz.constructor,
        (int) atrack->spec.stream_type,
        (int) atrack->spec.sample_rate_in_hz,
        (int) atrack->spec.channel_config,
        (int) atrack->spec.audio_format,
        (int) min_buffer_size,
        (int) atrack->spec.mode);
    if (!thiz || (*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_new: NewObject: Exception:");
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        free(atrack);
        return NULL;
    }

    atrack->min_buffer_size = min_buffer_size;
    atrack->spec.buffer_size_in_bytes = min_buffer_size;
    atrack->max_volume = audiotrack_get_max_volume(env);
    atrack->min_volume = audiotrack_get_min_volume(env);

    atrack->thiz = (*env)->NewGlobalRef(env, thiz);
    (*env)->DeleteLocalRef(env, thiz);

    // extra init
    float init_volume = 1.0f;
    init_volume = IJKMIN(init_volume, atrack->max_volume);
    init_volume = IJKMAX(init_volume, atrack->min_volume);
    ALOGI("SDL_Android_AudioTrack_new: init volume as %f/(%f,%f)", init_volume, atrack->min_volume, atrack->max_volume);
    audiotrack_set_stereo_volume(env, atrack, init_volume, init_volume);

    return atrack;
}

SDL_Android_AudioTrack *SDL_Android_AudioTrack_new_from_sdl_spec(JNIEnv *env, const SDL_AudioSpec *sdl_spec)
{
    SDL_Android_AudioTrack_Spec atrack_spec;

    SDL_Android_AudioTrack_get_default_spec(&atrack_spec);
    atrack_spec.sample_rate_in_hz = sdl_spec->freq;
    atrack_spec.channel_config = find_android_channel(sdl_spec->channels);
    atrack_spec.audio_format = find_android_format(sdl_spec->format);
    atrack_spec.buffer_size_in_bytes = sdl_spec->size;

    return SDL_Android_AudioTrack_new_from_spec(env, &atrack_spec);
}

void SDL_Android_AudioTrack_free(JNIEnv *env, SDL_Android_AudioTrack* atrack)
{
    if (atrack->byte_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->byte_buffer);
        atrack->byte_buffer = NULL;
    }
    atrack->byte_buffer_capacity = 0;

    if (atrack->float_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->float_buffer);
        atrack->float_buffer = NULL;
    }
    atrack->float_buffer_capacity = 0;

    if (atrack->thiz) {
        SDL_Android_AudioTrack_release(env, atrack);
        (*env)->DeleteGlobalRef(env, atrack->thiz);
        atrack->thiz = NULL;
    }

    free(atrack);
}

void SDL_Android_AudioTrack_get_target_spec(SDL_Android_AudioTrack *atrack, SDL_AudioSpec *sdl_spec)
{
    SDL_Android_AudioTrack_Spec *atrack_spec = &atrack->spec;

    sdl_spec->freq = atrack_spec->sample_rate_in_hz;
    sdl_spec->channels = find_sdl_channel(atrack_spec->channel_config);
    sdl_spec->format = find_sdl_format(atrack_spec->audio_format);
    sdl_spec->size = atrack_spec->buffer_size_in_bytes;
    sdl_spec->silence = 0;
    sdl_spec->padding = 0;
}

int SDL_Android_AudioTrack_get_min_buffer_size(SDL_Android_AudioTrack* atrack)
{
    return atrack->min_buffer_size;
}

void SDL_Android_AudioTrack_play(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("SDL_Android_AudioTrack_play");
    (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.play);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_play: play: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void SDL_Android_AudioTrack_pause(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("SDL_Android_AudioTrack_pause");
    (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.pause);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_pause: pause: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void SDL_Android_AudioTrack_flush(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("SDL_Android_AudioTrack_flush");
    (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.flush);
    SDLTRACE("SDL_Android_AudioTrack_flush()=void");
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_flush: flush: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void SDL_Android_AudioTrack_stop(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("SDL_Android_AudioTrack_stop");
    (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.stop);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_stop: stop: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

void SDL_Android_AudioTrack_release(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("SDL_Android_AudioTrack_release");
    (*env)->CallVoidMethod(env, atrack->thiz, g_clazz.release);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("SDL_Android_AudioTrack_release: release: Exception:");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return;
    }
}

int SDL_Android_AudioTrack_reserve_byte_buffer(JNIEnv *env, SDL_Android_AudioTrack *atrack, int size_in_byte)
{
    if (atrack->byte_buffer && size_in_byte <= atrack->byte_buffer_capacity)
        return size_in_byte;

    if (atrack->byte_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->byte_buffer);
        atrack->byte_buffer = NULL;
        atrack->byte_buffer_capacity = 0;
    }

    int capacity = IJKMAX(size_in_byte, atrack->min_buffer_size);
    jbyteArray byte_buffer = (*env)->NewByteArray(env, capacity);
    if (!byte_buffer || (*env)->ExceptionCheck(env)) {
        ALOGE("%s: NewByteArray: Exception:", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    atrack->byte_buffer_capacity = capacity;
    atrack->byte_buffer = (*env)->NewGlobalRef(env, byte_buffer);
    (*env)->DeleteLocalRef(env, byte_buffer);
    return capacity;
}

int SDL_Android_AudioTrack_write_byte(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_byte)
{
    if (size_in_byte <= 0)
        return size_in_byte;

    int reserved = SDL_Android_AudioTrack_reserve_byte_buffer(env, atrack, size_in_byte);
    if (reserved < size_in_byte) {
        ALOGE("%s failed %d < %d\n", __func__, reserved, size_in_byte);
        return -1;
    }

    (*env)->SetByteArrayRegion(env, atrack->byte_buffer, 0, (int)size_in_byte, (jbyte*) data);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: SetByteArrayRegion: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    int retval = (*env)->CallIntMethod(env, atrack->thiz, g_clazz.write_byte,
        atrack->byte_buffer, 0, (int)size_in_byte);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: write_byte: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}

int SDL_Android_AudioTrack_reserve_float_buffer(JNIEnv *env, SDL_Android_AudioTrack *atrack, int size_in_float)
{
    if (atrack->float_buffer && size_in_float <= atrack->float_buffer_capacity)
        return size_in_float;

    if (atrack->float_buffer) {
        (*env)->DeleteGlobalRef(env, atrack->float_buffer);
        atrack->float_buffer = NULL;
        atrack->float_buffer_capacity = 0;
    }

    int capacity = IJKMAX(size_in_float, ((atrack->min_buffer_size + sizeof(jfloat) - 1) / sizeof(jfloat)));
    jbyteArray float_buffer = (*env)->NewFloatArray(env, capacity);
    if (!float_buffer || (*env)->ExceptionCheck(env)) {
        ALOGE("%s: NewByteArray: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    atrack->float_buffer_capacity = capacity;
    atrack->float_buffer = (*env)->NewGlobalRef(env, float_buffer);
    (*env)->DeleteLocalRef(env, float_buffer);
    return capacity;
}

int SDL_Android_AudioTrack_write_float(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_float)
{
    if (size_in_float <= 0)
        return size_in_float;

    int reserved = SDL_Android_AudioTrack_reserve_float_buffer(env, atrack, size_in_float);
    if (reserved < size_in_float) {
        ALOGE("%s failed %d < %d\n", __func__, reserved, size_in_float);
        return -1;
    }

    (*env)->SetFloatArrayRegion(env, atrack->float_buffer, 0, (int)size_in_float, (jfloat*) data);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: SetByteArrayRegion: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    int retval = (*env)->CallIntMethod(env, atrack->thiz, g_clazz.write_float,
        atrack->float_buffer, 0, (int)size_in_float, (int)WRITE_BLOCKING);
    if ((*env)->ExceptionCheck(env)) {
        ALOGE("%s: write_byte: Exception:\n", __func__);
        if ((*env)->ExceptionCheck(env)) {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
        }
        return -1;
    }

    return retval;
}

int SDL_Android_AudioTrack_write(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_byte)
{
    int ret = 0;
    if (atrack->spec.audio_format == ENCODING_PCM_FLOAT) {
        ret = SDL_Android_AudioTrack_write_float(env, atrack, data, size_in_byte / sizeof(jfloat));
        if (ret > 0)
            ret *= sizeof(jfloat);
    } else {
        ret = SDL_Android_AudioTrack_write_byte(env, atrack, data, size_in_byte);
    }
    return ret;
}
