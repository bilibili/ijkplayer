/*****************************************************************************
 * android_audiotrack.h
 *****************************************************************************
 *
 * Copyright (c) 2013 Bilibili
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

#ifndef IJKSDL_ANDROID__ANDROID_AUDIOTRACK_H
#define IJKSDL_ANDROID__ANDROID_AUDIOTRACK_H

#include <stdint.h>
#include <jni.h>
#include "../ijksdl_audio.h"
#include "../ijksdl_aout.h"

#define AUDIOTRACK_PLAYBACK_MAXSPEED (2)

typedef struct SDL_Android_AudioTrack_Spec {
    enum StreamType {
        STREAM_VOICE_CALL = 0,
        STREAM_SYSTEM = 1,
        STREAM_RING = 2,
        STREAM_MUSIC = 3,
        STREAM_ALARM = 4,
        STREAM_NOTIFICATION = 5,
    } stream_type;

    int sample_rate_in_hz;

    enum ChannelConfig {
        CHANNEL_OUT_INVALID = 0x0,
        CHANNEL_OUT_DEFAULT = 0x1, /* f-l */
        CHANNEL_OUT_MONO = 0x4, /* f-l, f-r */
        CHANNEL_OUT_STEREO = 0xc, /* f-l, f-r, b-l, b-r */
        CHANNEL_OUT_QUAD = 0xcc, /* f-l, f-r, b-l, b-r */
        CHANNEL_OUT_SURROUND = 0x41c, /* f-l, f-r, f-c, b-c */
        CHANNEL_OUT_5POINT1 = 0xfc, /* f-l, f-r, b-l, b-r, f-c, low */
        CHANNEL_OUT_7POINT1 = 0x3fc, /* f-l, f-r, b-l, b-r, f-c, low, f-lc, f-rc */

        CHANNEL_OUT_FRONT_LEFT = 0x4,
        CHANNEL_OUT_FRONT_RIGHT = 0x8,
        CHANNEL_OUT_BACK_LEFT = 0x40,
        CHANNEL_OUT_BACK_RIGHT = 0x80,
        CHANNEL_OUT_FRONT_CENTER = 0x10,
        CHANNEL_OUT_LOW_FREQUENCY = 0x20,
        CHANNEL_OUT_FRONT_LEFT_OF_CENTER = 0x100,
        CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x200,
        CHANNEL_OUT_BACK_CENTER = 0x400,
    } channel_config;

    enum AudioFormat {
        ENCODING_INVALID = 0,
        ENCODING_DEFAULT = 1,
        ENCODING_PCM_16BIT = 2, // signed, guaranteed to be supported by devices.
        ENCODING_PCM_8BIT = 3, // unsigned, not guaranteed to be supported by devices.
        ENCODING_PCM_FLOAT = 4, // single-precision floating-point per sample
    } audio_format;
    int buffer_size_in_bytes;

    enum Mode {
        MODE_STATIC = 0,
        MODE_STREAM = 1,
    } mode;

    enum WriteMode {
        WRITE_BLOCKING     = 0,
        WRITE_NON_BLOCKING = 1,
    } write_mode; // not used

    // extra field
    int sdl_samples;
} SDL_Android_AudioTrack_Spec;

typedef struct SDL_Android_AudioTrack SDL_Android_AudioTrack;

SDL_Android_AudioTrack *SDL_Android_AudioTrack_new_from_spec(JNIEnv *env, SDL_Android_AudioTrack_Spec *spec);
SDL_Android_AudioTrack *SDL_Android_AudioTrack_new_from_sdl_spec(JNIEnv *env, const SDL_AudioSpec *sdl_spec);
void SDL_Android_AudioTrack_free(JNIEnv *env, SDL_Android_AudioTrack* atrack);

void SDL_Android_AudioTrack_get_target_spec(SDL_Android_AudioTrack* atrack, SDL_AudioSpec *spec);
int SDL_Android_AudioTrack_get_min_buffer_size(SDL_Android_AudioTrack* atrack);
int audiotrack_get_native_output_sample_rate(JNIEnv *env/* = NULL */);

void SDL_Android_AudioTrack_play(JNIEnv *env, SDL_Android_AudioTrack *atrack);
void SDL_Android_AudioTrack_pause(JNIEnv *env, SDL_Android_AudioTrack *atrack);
void SDL_Android_AudioTrack_flush(JNIEnv *env, SDL_Android_AudioTrack *atrack);
void SDL_Android_AudioTrack_set_volume(JNIEnv *env, SDL_Android_AudioTrack *atrack, float left_volume, float right_volume);
void SDL_Android_AudioTrack_stop(JNIEnv *env, SDL_Android_AudioTrack *atrack);
void SDL_Android_AudioTrack_release(JNIEnv *env, SDL_Android_AudioTrack *atrack);
int SDL_Android_AudioTrack_write(JNIEnv *env, SDL_Android_AudioTrack *atrack, uint8_t *data, int size_in_byte);

int  SDL_Android_AudioTrack_getAudioSessionId(JNIEnv *env, SDL_Android_AudioTrack *atrack);
void SDL_Android_AudioTrack_setSpeed(JNIEnv *env, SDL_Android_AudioTrack *atrack, float speed);

#endif
