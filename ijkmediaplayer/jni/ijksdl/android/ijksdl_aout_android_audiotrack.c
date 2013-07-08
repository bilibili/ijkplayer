/*****************************************************************************
 * ijksdl_aout_android_audiotrack.c
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

#include "ijksdl_aout_android_audiotrack.h"

#include <stdbool.h>
#include <assert.h>
#include <jni.h>
#include "../ijksdl_inc_internal.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "ijksdl_android_jni.h"
#include "android_audiotrack.h"

#ifdef SDLTRACE
#undef SDLTRACE
#define SDLTRACE(...)
#endif

typedef struct SDL_Aout_Opaque {
    SDL_cond *wakeup_cond;
    SDL_mutex *wakeup_mutex;

    SDL_AudioSpec spec;
    SDL_AndroidAudioTrack* atrack;
    uint8_t *buffer;
    int buffer_size;

    volatile bool need_flush;
    volatile bool pause_on;
    volatile bool abort_request;

    SDL_Thread *audio_tid;
    SDL_Thread _audio_tid;
} SDL_Aout_Opaque;

int aout_thread_n(JNIEnv *env, SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_AndroidAudioTrack *atrack = opaque->atrack;
    SDL_AudioCallback audio_cblk = opaque->spec.callback;
    void *userdata = opaque->spec.userdata;
    uint8_t *buffer = opaque->buffer;
    int copy_size = 256;

    assert(atrack);
    assert(buffer);

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    while (!opaque->abort_request) {
        SDL_LockMutex(opaque->wakeup_mutex);
        if (!opaque->abort_request && opaque->pause_on)
            sdl_audiotrack_pause(env, atrack);
        while (!opaque->abort_request && opaque->pause_on)
            SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex, 1000);
        if (!opaque->abort_request && !opaque->pause_on)
            sdl_audiotrack_play(env, atrack);
        SDL_UnlockMutex(opaque->wakeup_mutex);

        audio_cblk(userdata, buffer, copy_size);
        if (opaque->need_flush) {
            sdl_audiotrack_flush(env, atrack);
            opaque->need_flush = false;
        }
        sdl_audiotrack_write_byte(env, atrack, buffer, copy_size);

        // TODO: 1 if callback return -1 or 0
    }

    sdl_audiotrack_free(env, atrack);
    return 0;
}

int aout_thread(void *arg)
{
    SDL_Aout *aout = arg;
    // SDL_Aout_Opaque *opaque = aout->opaque;
    JNIEnv *env = NULL;

    if (JNI_OK != SDL_AndroidJni_SetupThreadEnv(&env)) {
        ALOGE("aout_thread: SDL_AndroidJni_SetupEnv: failed");
        return -1;
    }

    return aout_thread_n(env, aout);
}

int aout_open_audio_n(JNIEnv *env, SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    assert(desired);
    SDL_Aout_Opaque *opaque = aout->opaque;

    opaque->spec = *desired;
    opaque->atrack = sdl_audiotrack_new_from_sdl_spec(env, desired);
    if (!opaque->atrack) {
        ALOGE("aout_open_audio_n: failed to new AudioTrcak()");
        return -1;
    }

    opaque->buffer_size = sdl_audiotrack_get_min_buffer_size(opaque->atrack);
    if (opaque->buffer_size <= 0) {
        ALOGE("aout_open_audio_n: failed to getMinBufferSize()");
        sdl_audiotrack_free(env, opaque->atrack);
        opaque->atrack = NULL;
        return -1;
    }

    opaque->buffer = malloc(opaque->buffer_size);
    if (!opaque->buffer) {
        ALOGE("aout_open_audio_n: failed to allocate buffer");
        sdl_audiotrack_free(env, opaque->atrack);
        opaque->atrack = NULL;
        return -1;
    }

    if (obtained) {
        sdl_audiotrack_get_target_spec(opaque->atrack, obtained);
        SDLTRACE("audio target format fmt:0x%x, channel:0x%x", (int)obtained->format, (int)obtained->channels);
    }

    opaque->pause_on = 1;
    opaque->abort_request = 0;
    opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread, aout);
    if (!opaque->audio_tid) {
        ALOGE("aout_open_audio_n: failed to create audio thread");
        sdl_audiotrack_free(env, opaque->atrack);
        opaque->atrack = NULL;
        return -1;
    }

    return 0;
}

int aout_open_audio(SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    // SDL_Aout_Opaque *opaque = aout->opaque;
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_AndroidJni_SetupThreadEnv(&env)) {
        ALOGE("aout_open_audio: AttachCurrentThread: failed");
        return -1;
    }

    return aout_open_audio_n(env, aout, desired, obtained);
}

void aout_pause_audio(SDL_Aout *aout, int pause_on)
{
    SDL_Aout_Opaque *opaque = aout->opaque;

    SDL_LockMutex(opaque->wakeup_mutex);
    SDLTRACE("aout_pause_audio(%d)", pause_on);
    opaque->pause_on = pause_on;
    if (!pause_on)
        SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

void aout_close_audio(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;

    SDL_LockMutex(opaque->wakeup_mutex);
    opaque->abort_request = true;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);

    SDL_WaitThread(opaque->audio_tid, NULL);

    opaque->audio_tid = NULL;
}

void aout_free_l(SDL_Aout *aout)
{
    if (!aout)
        return;

    aout_close_audio(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;
    if (opaque) {
        free(opaque->buffer);
        opaque->buffer = NULL;
        opaque->buffer_size = 0;

        SDL_DestroyCond(opaque->wakeup_cond);
        SDL_DestroyMutex(opaque->wakeup_mutex);
    }

    SDL_Aout_FreeInternal(aout);
}

SDL_Aout *SDL_AoutAndroid_CreateForAudioTrack()
{
    SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;

    SDL_Aout_Opaque *opaque = aout->opaque;
    opaque->wakeup_cond = SDL_CreateCond();
    opaque->wakeup_mutex = SDL_CreateMutex();

    aout->free_l = aout_free_l;
    aout->open_audio = aout_open_audio;
    aout->pause_audio = aout_pause_audio;
    aout->close_audio = aout_close_audio;

    return aout;
}

void SDL_Init_AoutAndroid(JNIEnv *env)
{

}
