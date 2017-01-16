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
#include "j4au/class/android/media/AudioTrack.util.h"
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
} SDL_Android_AudioTrack;

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

#define STREAM_MUSIC 3
int audiotrack_get_native_output_sample_rate(JNIEnv *env)
{
    if (!env) {
        if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
            ALOGE("%s: SetupThreadEnv failed", __func__);
            return -1;
        }
    }

    jint retval = J4AC_AudioTrack__getNativeOutputSampleRate(env, STREAM_MUSIC);
    if (J4A_ExceptionCheck__catchAll(env) || retval <= 0)
        return -1;

    return retval;
}

void SDL_Android_AudioTrack_set_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left_volume, float right_volume)
{
    J4AC_AudioTrack__setStereoVolume__catchAll(env, atrack->thiz, left_volume, right_volume);
}

SDL_Android_AudioTrack *SDL_Android_AudioTrack_new_from_spec(JNIEnv *env, SDL_Android_AudioTrack_Spec *spec)
{
    assert(spec);

    switch (spec->channel_config) {
    case CHANNEL_OUT_MONO:
        ALOGI("SDL_Android_AudioTrack: %s", "CHANNEL_OUT_MONO");
        break;
    case CHANNEL_OUT_STEREO:
        ALOGI("SDL_Android_AudioTrack: %s", "CHANNEL_OUT_STEREO");
        break;
    default:
        ALOGE("%s: invalid channel %d", __func__, spec->channel_config);
        return NULL;
    }

    switch (spec->audio_format) {
    case ENCODING_PCM_16BIT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_16BIT");
        break;
    case ENCODING_PCM_8BIT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_8BIT");
        break;
#if 0
    case ENCODING_PCM_FLOAT:
        ALOGI("SDL_Android_AudioTrack: %s", "ENCODING_PCM_FLOAT");
        if (sdk_int < IJK_API_21_LOLLIPOP) {
            ALOGI("SDL_Android_AudioTrack: %s need API 21 or above", "ENCODING_PCM_FLOAT");
            return NULL;
        }
        break;
#endif
    default:
        ALOGE("%s: invalid format %d", __func__, spec->audio_format);
        return NULL;
    }

    if (spec->sample_rate_in_hz <= 0) {
        ALOGE("%s: invalid sample rate %d", __func__, spec->sample_rate_in_hz);
        return NULL;
    }

    SDL_Android_AudioTrack *atrack = (SDL_Android_AudioTrack*) mallocz(sizeof(SDL_Android_AudioTrack));
    if (!atrack) {
        ALOGE("%s: mallocz faild.\n", __func__);
        return NULL;
    }
    atrack->spec = *spec;

    // libswresample is ugly, depending on native resampler
    while (atrack->spec.sample_rate_in_hz < 4000) {
        atrack->spec.sample_rate_in_hz *= 2;
    }
    while (atrack->spec.sample_rate_in_hz > 48000) {
        atrack->spec.sample_rate_in_hz /= 2;   
    }

    int min_buffer_size = J4AC_AudioTrack__getMinBufferSize(env,
        atrack->spec.sample_rate_in_hz,
        atrack->spec.channel_config,
        atrack->spec.audio_format);
    if (J4A_ExceptionCheck__catchAll(env) || min_buffer_size <= 0) {
        ALOGE("%s: J4AC_AudioTrack__getMinBufferSize: return %d:", __func__, min_buffer_size);
        free(atrack);
        return NULL;
    }

    // for fast playback
    min_buffer_size *= AUDIOTRACK_PLAYBACK_MAXSPEED;

    atrack->thiz = J4AC_AudioTrack__AudioTrack__asGlobalRef__catchAll(env, 
        atrack->spec.stream_type,
        atrack->spec.sample_rate_in_hz,
        atrack->spec.channel_config,
        atrack->spec.audio_format,
        min_buffer_size,
        atrack->spec.mode);
    if (!atrack->thiz) {
        free(atrack);
        return NULL;
    }

    atrack->min_buffer_size = min_buffer_size;
    atrack->spec.buffer_size_in_bytes = min_buffer_size;
    // atrack->max_volume = J4AC_AudioTrack__getMaxVolume__catchAll(env);
    // atrack->min_volume = J4AC_AudioTrack__getMinVolume__catchAll(env);
    atrack->max_volume = 1.0f;
    atrack->min_volume = 0.0f;

    // extra init
    float init_volume = 1.0f;
    init_volume = IJKMIN(init_volume, atrack->max_volume);
    init_volume = IJKMAX(init_volume, atrack->min_volume);
    ALOGI("%s: init volume as %f/(%f,%f)", __func__, init_volume, atrack->min_volume, atrack->max_volume);
    J4AC_AudioTrack__setStereoVolume__catchAll(env, atrack->thiz, init_volume, init_volume);

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
    J4A_DeleteGlobalRef__p(env, &atrack->byte_buffer);
    atrack->byte_buffer_capacity = 0;

    if (atrack->thiz) {
        J4AC_AudioTrack__release(env, atrack->thiz);
        J4A_DeleteGlobalRef__p(env, &atrack->thiz);
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
    SDLTRACE("%s", __func__);
    J4AC_AudioTrack__play__catchAll(env, atrack->thiz);
}

void SDL_Android_AudioTrack_pause(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("%s", __func__);
    J4AC_AudioTrack__pause__catchAll(env, atrack->thiz);
}

void SDL_Android_AudioTrack_flush(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("%s", __func__);
    J4AC_AudioTrack__flush__catchAll(env, atrack->thiz);
}

void SDL_Android_AudioTrack_stop(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("%s", __func__);
    J4AC_AudioTrack__stop__catchAll(env, atrack->thiz);
}

int SDL_Android_AudioTrack_reserve_byte_buffer(JNIEnv *env, SDL_Android_AudioTrack *atrack, int size_in_byte)
{
    if (atrack->byte_buffer && size_in_byte <= atrack->byte_buffer_capacity)
        return size_in_byte;

    J4A_DeleteGlobalRef__p(env, &atrack->byte_buffer);
    atrack->byte_buffer_capacity = 0;

    int capacity = IJKMAX(size_in_byte, atrack->min_buffer_size);
    atrack->byte_buffer = J4A_NewByteArray__asGlobalRef__catchAll(env, capacity);
    if (!atrack->byte_buffer)
        return -1;

    atrack->byte_buffer_capacity = capacity;
    return capacity;
}

int SDL_Android_AudioTrack_write(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_byte)
{
    if (size_in_byte <= 0)
        return size_in_byte;

    int reserved = SDL_Android_AudioTrack_reserve_byte_buffer(env, atrack, size_in_byte);
    if (reserved < size_in_byte) {
        ALOGE("%s failed %d < %d\n", __func__, reserved, size_in_byte);
        return -1;
    }

    (*env)->SetByteArrayRegion(env, atrack->byte_buffer, 0, (int)size_in_byte, (jbyte*) data);
    if (J4A_ExceptionCheck__catchAll(env))
        return -1;

    int retval = J4AC_AudioTrack__write(env, atrack->thiz, atrack->byte_buffer, 0, (int)size_in_byte);
    if (J4A_ExceptionCheck__catchAll(env))
        return -1;

    return retval;
}

int SDL_Android_AudioTrack_getAudioSessionId(JNIEnv *env, SDL_Android_AudioTrack *atrack)
{
    SDLTRACE("%s", __func__);
    int audioSessionId = J4AC_AudioTrack__getAudioSessionId(env, atrack->thiz);
    if (J4A_ExceptionCheck__catchAll(env))
        return 0;

    return audioSessionId;
}

void SDL_Android_AudioTrack_setSpeed(JNIEnv *env, SDL_Android_AudioTrack *atrack, float speed)
{
    J4AC_AudioTrack__setSpeed(env, atrack->thiz, speed);
    if (J4A_ExceptionCheck__catchAll(env))
        return;

    return;
}
