/*****************************************************************************
 * ijksdl_aout_android_opensles.c
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

#include <stdbool.h>
#include <assert.h>
#include <jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../ijksdl_inc_internal.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "ijksdl_android_jni.h"

#ifdef SDLTRACE
#undef SDLTRACE
#define SDLTRACE(...)
//#define SDLTRACE ALOGW
#endif

#define OPENSLES_BUFFERS 255 /* maximum number of buffers */
#define OPENSLES_BUFLEN  10 /* ms */

typedef struct SDL_Aout_Opaque {
    SDL_mutex  *mutex;

    SDL_cond   *wakeup_cond;
    SDL_mutex  *wakeup_mutex;

    SDL_Thread *audio_tid;
    SDL_Thread _audio_tid;

    SDL_AudioSpec    spec;
    SLDataFormat_PCM format_pcm;
    int              bytes_per_frame;
    int              frame_per_copy;
    int              bytes_per_copy;

    SLObjectItf                     slObject;
    SLEngineItf                     slEngine;

    SLObjectItf                     slOutputMixObject;

    SLObjectItf                     slPlayerObject;
    SLAndroidSimpleBufferQueueItf   slBufferQueueItf;
    SLVolumeItf                     slVolumeItf;
    SLPlayItf                       slPlayItf;

    volatile bool  abort_request;
    volatile bool  pause_on;
    volatile bool  need_flush;
    volatile bool  is_running;

    volatile int   buffer_ready_count;
    uint8_t       *buffer;
    size_t         buffer_capacity;
} SDL_Aout_Opaque;

#define CHECK_OPENSL_ERROR(ret__, ...) \
    do { \
    	if ((ret__) != SL_RESULT_SUCCESS) \
    	{ \
    		ALOGE(__VA_ARGS__); \
    		goto fail; \
    	} \
    } while (0)

#define CHECK_COND_ERROR(cond__, ...) \
    do { \
        if (!(cond__)) \
        { \
            ALOGE(__VA_ARGS__); \
            goto fail; \
        } \
    } while (0)

static int aout_thread_n(SDL_Aout *aout)
{
    SDL_Aout_Opaque               *opaque           = aout->opaque;
    SLPlayItf                      slPlayItf        = opaque->slPlayItf;
    SLAndroidSimpleBufferQueueItf  slBufferQueueItf = opaque->slBufferQueueItf;
    SDL_AudioCallback              audio_cblk       = opaque->spec.callback;
    void                          *userdata         = opaque->spec.userdata;
    uint8_t                       *buffer           = opaque->buffer;

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    if (!opaque->abort_request && !opaque->pause_on)
        (*opaque->slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_PLAYING);

    while (!opaque->abort_request) {
        SDL_LockMutex(opaque->wakeup_mutex);
        if (!opaque->abort_request && opaque->pause_on) {
            (*slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_PAUSED);
            while (!opaque->abort_request && opaque->pause_on) {
                SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex, 1000);
            }
            if (!opaque->abort_request && !opaque->pause_on)
                (*slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_PLAYING);
        }
        if (opaque->need_flush) {
            opaque->need_flush = 0;
            (*slBufferQueueItf)->Clear(opaque->slBufferQueueItf);
        }
#if 0
        if (opaque->need_set_volume) {
            opaque->need_set_volume = 0;
            // FIXME: set volume here
        }
#endif
        SDL_UnlockMutex(opaque->wakeup_mutex);

        audio_cblk(userdata, opaque->buffer, opaque->buffer_capacity);
        if (opaque->need_flush) {
            (*slBufferQueueItf)->Clear(opaque->slBufferQueueItf);
            opaque->need_flush = false;
        }

        if (opaque->need_flush) {
            opaque->need_flush = 0;
            (*slBufferQueueItf)->Clear(opaque->slBufferQueueItf);
        } else {
            while (!opaque->abort_request && opaque->buffer_ready_count <= 0) {
                SDL_Delay(1);
            }

            if (opaque->abort_request)
                break;

            if (opaque->buffer_ready_count > 0) {
                int ret = (*slBufferQueueItf)->Enqueue(slBufferQueueItf, opaque->buffer, opaque->buffer_capacity);
                if (ret == SL_RESULT_SUCCESS) {
                    __sync_sub_and_fetch(&opaque->buffer_ready_count, 1);
                } else if (ret == SL_RESULT_BUFFER_INSUFFICIENT) {
                    ALOGE("SL_RESULT_BUFFER_INSUFFICIENT\n");
                    __sync_sub_and_fetch(&opaque->buffer_ready_count, 1);
                } else {
                    ALOGE("slBufferQueueItf->Enqueue() = %d\n", ret);
                    break;
                }
            }
        }

        // TODO: 1 if callback return -1 or 0
    }

    return 0;
}

static int aout_thread(void *arg)
{
    return aout_thread_n(arg);
}

static void aout_opensles_callback(SLAndroidSimpleBufferQueueItf caller, void *pContext)
{
    SDLTRACE("%s\n", __func__);
    SDL_Aout        *aout   = pContext;
    SDL_Aout_Opaque *opaque = aout->opaque;

    if (opaque) {
        opaque->is_running = true;
        int value = __sync_fetch_and_add(&opaque->buffer_ready_count, 1);
        // ALOGI("callback: %d", value + 1);
    }
}

static void aout_close_audio(SDL_Aout *aout)
{
    SDLTRACE("aout_close_audio()\n");
    SDL_Aout_Opaque *opaque = aout->opaque;
    if (!opaque->slPlayerObject)
        return;

    SDL_LockMutex(opaque->wakeup_mutex);
    opaque->abort_request = true;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);

    SDL_WaitThread(opaque->audio_tid, NULL);
    opaque->audio_tid = NULL;

    (*opaque->slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_STOPPED);
    (*opaque->slBufferQueueItf)->Clear(opaque->slBufferQueueItf);

    opaque->slBufferQueueItf = NULL;
    opaque->slVolumeItf      = NULL;
    opaque->slPlayItf        = NULL;

    (*opaque->slPlayerObject)->Destroy(opaque->slPlayerObject);
    opaque->slPlayerObject = NULL;
}

static void aout_free_l(SDL_Aout *aout)
{
    SDLTRACE("%s\n", __func__);
    if (!aout)
        return;

    aout_close_audio(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;

    (*opaque->slOutputMixObject)->Destroy(opaque->slOutputMixObject);
    opaque->slOutputMixObject = NULL;

    opaque->slEngine = NULL;
    (*opaque->slObject)->Destroy(opaque->slObject);
    opaque->slObject = NULL;

    SDL_Aout_FreeInternal(aout);
}

static int aout_open_audio(SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    SDLTRACE("%s\n", __func__);
    assert(desired);
    SDLTRACE("aout_open_audio()\n");
    SDL_Aout_Opaque  *opaque     = aout->opaque;
    SLEngineItf       slEngine   = opaque->slEngine;
    SLDataFormat_PCM *format_pcm = &opaque->format_pcm;
    int               ret = 0;

    opaque->spec = *desired;

    // config audio src
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        OPENSLES_BUFFERS
    };

    CHECK_COND_ERROR((desired->format == AUDIO_S16SYS), "%s: not AUDIO_S16SYS", __func__);
    CHECK_COND_ERROR((desired->channels == 2), "%s: not 2 channel", __func__);
    CHECK_COND_ERROR((desired->freq >= 8000 && desired->freq <= 48000), "%s: unsupport freq %d Hz", __func__, desired->freq);
    format_pcm->formatType       = SL_DATAFORMAT_PCM;
    format_pcm->numChannels      = desired->channels;
    format_pcm->samplesPerSec    = desired->freq * 1000; // milli Hz
    // format_pcm->numChannels      = 2;
    // format_pcm->samplesPerSec    = SL_SAMPLINGRATE_44_1;

    format_pcm->bitsPerSample    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm->containerSize    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm->channelMask      = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    format_pcm->endianness       = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audio_source = {&loc_bufq, format_pcm};

    // config audio sink
    SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX,
        opaque->slOutputMixObject
    };
    SLDataSink audio_sink = {&loc_outmix, NULL};

    SLObjectItf slPlayerObject = NULL;
    const SLInterfaceID ids2[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAY };
    static const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    ret = (*slEngine)->CreateAudioPlayer(slEngine, &slPlayerObject, &audio_source,
                                &audio_sink, sizeof(ids2) / sizeof(*ids2),
                                ids2, req2);
    CHECK_OPENSL_ERROR(ret, "%s: slEngine->CreateAudioPlayer() failed", __func__);
    opaque->slPlayerObject = slPlayerObject;

    ret = (*slPlayerObject)->Realize(slPlayerObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR(ret, "%s: slPlayerObject->Realize() failed", __func__);

    ret = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_PLAY, &opaque->slPlayItf);
    CHECK_OPENSL_ERROR(ret, "%s: slPlayerObject->GetInterface(SL_IID_PLAY) failed", __func__);

    ret = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_VOLUME, &opaque->slVolumeItf);
    CHECK_OPENSL_ERROR(ret, "%s: slPlayerObject->GetInterface(SL_IID_VOLUME) failed", __func__);

    ret = (*slPlayerObject)->GetInterface(slPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &opaque->slBufferQueueItf);
    CHECK_OPENSL_ERROR(ret, "%s: slPlayerObject->GetInterface(SL_IID_ANDROIDSIMPLEBUFFERQUEUE) failed", __func__);

    ret = (*opaque->slBufferQueueItf)->RegisterCallback(opaque->slBufferQueueItf, aout_opensles_callback, (void*)aout);
    CHECK_OPENSL_ERROR(ret, "%s: slBufferQueueItf->RegisterCallback() failed", __func__);

    // set the player's state to playing
    // ret = (*opaque->slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_PLAYING);
    // CHECK_OPENSL_ERROR(ret, "%s: slBufferQueueItf->slPlayItf() failed", __func__);

    opaque->bytes_per_frame = desired->channels * format_pcm->bitsPerSample / 8;
    //opaque->frame_per_copy  = desired->freq / 100; // 10 ms per copy
    opaque->frame_per_copy  = desired->freq;
    opaque->bytes_per_copy  = opaque->bytes_per_frame * opaque->frame_per_copy;
    opaque->buffer_capacity = opaque->bytes_per_copy;
    ALOGI("OpenSL-ES: frame_per_copy  = %d\n", (int)opaque->frame_per_copy);
    ALOGI("OpenSL-ES: bytes_per_frame = %d\n", (int)opaque->bytes_per_frame);
    ALOGI("OpenSL-ES: bytes_per_copy  = %d\n", (int)opaque->bytes_per_copy);
    ALOGI("OpenSL-ES: buffer_capacity = %d\n", (int)opaque->buffer_capacity);
    opaque->buffer          = malloc(opaque->buffer_capacity);
    CHECK_COND_ERROR(opaque->buffer, "%s: failed to alloc buffer", __func__);

    // (*opaque->slPlayItf)->SetPositionUpdatePeriod(opaque->slPlayItf, 1000);

    // enqueue empty buffer to start play
    opaque->buffer_ready_count = 0;
    memset(opaque->buffer, 0, opaque->buffer_capacity);
    for(int i = 0; i < OPENSLES_BUFFERS; ++i) {
        ret = (*opaque->slBufferQueueItf)->Enqueue(opaque->slBufferQueueItf, opaque->buffer, opaque->buffer_capacity);
        CHECK_OPENSL_ERROR(ret, "%s: slBufferQueueItf->Enqueue(000...) failed", __func__);
    }

    opaque->pause_on = 1;
    opaque->abort_request = 0;
    opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread, aout, "ff_aout_opensles");
    CHECK_COND_ERROR(opaque->audio_tid, "%s: failed to SDL_CreateThreadEx", __func__);

    if (obtained)
        *obtained = *desired;

    return opaque->buffer_capacity;
fail:
    aout_close_audio(aout);
    return 0;
}

static void aout_pause_audio(SDL_Aout *aout, int pause_on)
{
    SDL_Aout_Opaque *opaque = aout->opaque;

    SDL_LockMutex(opaque->wakeup_mutex);
    SDLTRACE("aout_pause_audio(%d)", pause_on);
    opaque->pause_on = pause_on;
    if (!pause_on)
        SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

static void aout_flush_audio(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_LockMutex(opaque->wakeup_mutex);
    SDLTRACE("aout_flush_audio()");
    opaque->need_flush = 1;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

SDL_Aout *SDL_AoutAndroid_CreateForOpenSLES()
{
    SDLTRACE("%s\n", __func__);
    SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;

    SDL_Aout_Opaque *opaque = aout->opaque;
    opaque->wakeup_cond = SDL_CreateCond();
    opaque->wakeup_mutex = SDL_CreateMutex();

    int ret = 0;

    SLObjectItf slObject = NULL;
    ret = slCreateEngine(&slObject, 0, NULL, 0, NULL, NULL);
    CHECK_OPENSL_ERROR(ret, "%s: slCreateEngine() failed", __func__);
    opaque->slObject = slObject;

    ret = (*slObject)->Realize(slObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR(ret, "%s: slObject->Realize() failed", __func__);

    SLEngineItf slEngine = NULL;
    ret = (*slObject)->GetInterface(slObject, SL_IID_ENGINE, &slEngine);
    CHECK_OPENSL_ERROR(ret, "%s: slObject->GetInterface() failed", __func__);
    opaque->slEngine = slEngine;

    SLObjectItf slOutputMixObject = NULL;
    const SLInterfaceID ids1[] = {SL_IID_VOLUME};
    const SLboolean req1[] = {SL_BOOLEAN_FALSE};
    ret = (*slEngine)->CreateOutputMix(slEngine, &slOutputMixObject, 1, ids1, req1);
    CHECK_OPENSL_ERROR(ret, "%s: slEngine->CreateOutputMix() failed", __func__);
    opaque->slOutputMixObject = slOutputMixObject;

    ret = (*slOutputMixObject)->Realize(slOutputMixObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR(ret, "%s: slOutputMixObject->Realize() failed", __func__);

    aout->free_l = aout_free_l;
    aout->open_audio = aout_open_audio;
    aout->pause_audio = aout_pause_audio;
    aout->flush_audio = aout_flush_audio;
    aout->close_audio = aout_close_audio;

    return aout;
fail:
    aout_free_l(aout);
    return NULL;
}
