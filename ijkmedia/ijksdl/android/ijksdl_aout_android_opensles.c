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
#include <math.h>
#include <inttypes.h>
#include <jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "../ijksdl_inc_internal.h"
#include "../ijksdl_thread.h"
#include "../ijksdl_aout_internal.h"
#include "ijksdl_android_jni.h"
#include "android_audiotrack.h"

#ifdef SDLTRACE
#undef SDLTRACE
#define SDLTRACE(...)
//#define SDLTRACE ALOGW
#endif

#define OPENSLES_BUFFERS 255 /* maximum number of buffers */
#define OPENSLES_BUFLEN  10 /* ms */

static SDL_Class g_opensles_class = {
    .name = "OpenSLES",
};

typedef struct SDL_Aout_Opaque {
    SDL_cond   *wakeup_cond;
    SDL_mutex  *wakeup_mutex;

    SDL_Thread *audio_tid;
    SDL_Thread _audio_tid;

    SDL_AudioSpec    spec;
    SLDataFormat_PCM format_pcm;
    int              bytes_per_frame;
    int              milli_per_buffer;
    int              frames_per_buffer;
    int              bytes_per_buffer;

    SLObjectItf                     slObject;
    SLEngineItf                     slEngine;

    SLObjectItf                     slOutputMixObject;

    SLObjectItf                     slPlayerObject;
    SLAndroidSimpleBufferQueueItf   slBufferQueueItf;
    SLVolumeItf                     slVolumeItf;
    SLPlayItf                       slPlayItf;

    volatile bool  need_set_volume;
    volatile float left_volume;
    volatile float right_volume;

    volatile bool  abort_request;
    volatile bool  pause_on;
    volatile bool  need_flush;
    volatile bool  is_running;

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

static inline SLmillibel android_amplification_to_sles(float volumeLevel) {
    // FIXME use the FX Framework conversions
    if (volumeLevel < 0.00000001)
        return SL_MILLIBEL_MIN;

    SLmillibel mb = lroundf(2000.f * log10f(volumeLevel));
    if (mb < SL_MILLIBEL_MIN)
        mb = SL_MILLIBEL_MIN;
    else if (mb > 0)
        mb = 0; /* maximum supported level could be higher: GetMaxVolumeLevel */
    return mb;
}

static int aout_thread_n(SDL_Aout *aout)
{
    SDL_Aout_Opaque               *opaque           = aout->opaque;
    SLPlayItf                      slPlayItf        = opaque->slPlayItf;
    SLAndroidSimpleBufferQueueItf  slBufferQueueItf = opaque->slBufferQueueItf;
    SLVolumeItf                    slVolumeItf      = opaque->slVolumeItf;
    SDL_AudioCallback              audio_cblk       = opaque->spec.callback;
    void                          *userdata         = opaque->spec.userdata;
    uint8_t                       *next_buffer      = NULL;
    int                            next_buffer_index = 0;
    size_t                         bytes_per_buffer = opaque->bytes_per_buffer;

    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    if (!opaque->abort_request && !opaque->pause_on)
        (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);

    while (!opaque->abort_request) {
        SLAndroidSimpleBufferQueueState slState = {0};

        SLresult slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
        if (slRet != SL_RESULT_SUCCESS) {
            ALOGE("%s: slBufferQueueItf->GetState() failed\n", __func__);
            SDL_UnlockMutex(opaque->wakeup_mutex);
        }

        SDL_LockMutex(opaque->wakeup_mutex);
        if (!opaque->abort_request && (opaque->pause_on || slState.count >= OPENSLES_BUFFERS)) {
            while (!opaque->abort_request && (opaque->pause_on || slState.count >= OPENSLES_BUFFERS)) {
                if (!opaque->pause_on) {
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
                }
                SDL_CondWaitTimeout(opaque->wakeup_cond, opaque->wakeup_mutex, 1000);
                SLresult slRet = (*slBufferQueueItf)->GetState(slBufferQueueItf, &slState);
                if (slRet != SL_RESULT_SUCCESS) {
                    ALOGE("%s: slBufferQueueItf->GetState() failed\n", __func__);
                    SDL_UnlockMutex(opaque->wakeup_mutex);
                }

                if (opaque->pause_on)
                    (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PAUSED);
            }
            if (!opaque->abort_request && !opaque->pause_on) {
                (*slPlayItf)->SetPlayState(slPlayItf, SL_PLAYSTATE_PLAYING);
            }
        }
        if (opaque->need_flush) {
            opaque->need_flush = 0;
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
        }
#if 0
        if (opaque->need_set_volume) {
            opaque->need_set_volume = 0;
            // FIXME: set volume here
        }
#endif
        if (opaque->need_set_volume) {
            opaque->need_set_volume = 0;
            SLmillibel level = android_amplification_to_sles((opaque->left_volume + opaque->right_volume) / 2);
            ALOGI("slVolumeItf->SetVolumeLevel((%f, %f) -> %d)\n", opaque->left_volume, opaque->right_volume, (int)level);
            slRet = (*slVolumeItf)->SetVolumeLevel(slVolumeItf, level);
            if (slRet != SL_RESULT_SUCCESS) {
                ALOGE("slVolumeItf->SetVolumeLevel failed %d\n", (int)slRet);
                // just ignore error
            }
        }
        SDL_UnlockMutex(opaque->wakeup_mutex);

        next_buffer = opaque->buffer + next_buffer_index * bytes_per_buffer;
        next_buffer_index = (next_buffer_index + 1) % OPENSLES_BUFFERS;
        audio_cblk(userdata, next_buffer, bytes_per_buffer);
        if (opaque->need_flush) {
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
            opaque->need_flush = false;
        }

        if (opaque->need_flush) {
            ALOGE("flush");
            opaque->need_flush = 0;
            (*slBufferQueueItf)->Clear(slBufferQueueItf);
        } else {
            slRet = (*slBufferQueueItf)->Enqueue(slBufferQueueItf, next_buffer, bytes_per_buffer);
            if (slRet == SL_RESULT_SUCCESS) {
                // do nothing
            } else if (slRet == SL_RESULT_BUFFER_INSUFFICIENT) {
                // don't retry, just pass through
                ALOGE("SL_RESULT_BUFFER_INSUFFICIENT\n");
            } else {
                ALOGE("slBufferQueueItf->Enqueue() = %d\n", (int)slRet);
                break;
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
        SDL_LockMutex(opaque->wakeup_mutex);
        opaque->is_running = true;
        SDL_CondSignal(opaque->wakeup_cond);
        SDL_UnlockMutex(opaque->wakeup_mutex);
    }
}

static void aout_close_audio(SDL_Aout *aout)
{
    SDLTRACE("aout_close_audio()\n");
    SDL_Aout_Opaque *opaque = aout->opaque;
    if (!opaque)
        return;

    SDL_LockMutex(opaque->wakeup_mutex);
    opaque->abort_request = true;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);

    SDL_WaitThread(opaque->audio_tid, NULL);
    opaque->audio_tid = NULL;

    freep((void **)&opaque->buffer);

    if (opaque->slPlayItf)
        (*opaque->slPlayItf)->SetPlayState(opaque->slPlayItf, SL_PLAYSTATE_STOPPED);
    if (opaque->slBufferQueueItf)
        (*opaque->slBufferQueueItf)->Clear(opaque->slBufferQueueItf);

    if (opaque->slBufferQueueItf)
        opaque->slBufferQueueItf = NULL;
    if (opaque->slVolumeItf)
        opaque->slVolumeItf      = NULL;
    if (opaque->slPlayItf)
        opaque->slPlayItf        = NULL;

    if (opaque->slPlayerObject) {
        (*opaque->slPlayerObject)->Destroy(opaque->slPlayerObject);
        opaque->slPlayerObject = NULL;
    }
}

static void aout_free_l(SDL_Aout *aout)
{
    SDLTRACE("%s\n", __func__);
    if (!aout)
        return;

    aout_close_audio(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;

    if (opaque->slOutputMixObject) {
        (*opaque->slOutputMixObject)->Destroy(opaque->slOutputMixObject);
        opaque->slOutputMixObject = NULL;
    }

    opaque->slEngine = NULL;
    if (opaque->slObject) {
        (*opaque->slObject)->Destroy(opaque->slObject);
        opaque->slObject = NULL;
    }

    SDL_DestroyCondP(&opaque->wakeup_cond);
    SDL_DestroyMutexP(&opaque->wakeup_mutex);

    SDL_Aout_FreeInternal(aout);
}

static int aout_open_audio(SDL_Aout *aout, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
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

    int native_sample_rate = audiotrack_get_native_output_sample_rate(NULL);
    ALOGI("OpenSL-ES: native sample rate %d Hz\n", native_sample_rate);

    CHECK_COND_ERROR((desired->format == AUDIO_S16SYS), "%s: not AUDIO_S16SYS", __func__);
    CHECK_COND_ERROR((desired->channels == 2 || desired->channels == 1), "%s: not 1,2 channel", __func__);
    CHECK_COND_ERROR((desired->freq >= 8000 && desired->freq <= 48000), "%s: unsupport freq %d Hz", __func__, desired->freq);
    if (SDL_Android_GetApiLevel() < IJK_API_21_LOLLIPOP &&
        native_sample_rate > 0 &&
        desired->freq < native_sample_rate) {
        // Don't try to play back a sample rate higher than the native one,
        // since OpenSL ES will try to use the fast path, which AudioFlinger
        // will reject (fast path can't do resampling), and will end up with
        // too small buffers for the resampling. See http://b.android.com/59453
        // for details. This bug is still present in 4.4. If it is fixed later
        // this workaround could be made conditional.
        //
        // by VLC/android_opensles.c
        ALOGW("OpenSL-ES: force resample %lu to native sample rate %d\n",
              (unsigned long) format_pcm->samplesPerSec / 1000,
              (int) native_sample_rate);
        format_pcm->samplesPerSec = native_sample_rate * 1000;
    }

    format_pcm->formatType       = SL_DATAFORMAT_PCM;
    format_pcm->numChannels      = desired->channels;
    format_pcm->samplesPerSec    = desired->freq * 1000; // milli Hz
    // format_pcm->numChannels      = 2;
    // format_pcm->samplesPerSec    = SL_SAMPLINGRATE_44_1;

    format_pcm->bitsPerSample    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm->containerSize    = SL_PCMSAMPLEFORMAT_FIXED_16;
    switch (desired->channels) {
    case 2:
        format_pcm->channelMask  = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        break;
    case 1:
        format_pcm->channelMask  = SL_SPEAKER_FRONT_CENTER;
        break;
    default:
        ALOGE("%s, invalid channel %d", __func__, desired->channels);
        goto fail;
    }
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
    static const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
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

    opaque->bytes_per_frame   = format_pcm->numChannels * format_pcm->bitsPerSample / 8;
    opaque->milli_per_buffer  = OPENSLES_BUFLEN;
    opaque->frames_per_buffer = opaque->milli_per_buffer * format_pcm->samplesPerSec / 1000000; // samplesPerSec is in milli
    opaque->bytes_per_buffer  = opaque->bytes_per_frame * opaque->frames_per_buffer;
    opaque->buffer_capacity   = OPENSLES_BUFFERS * opaque->bytes_per_buffer;
    ALOGI("OpenSL-ES: bytes_per_frame  = %d bytes\n",  (int)opaque->bytes_per_frame);
    ALOGI("OpenSL-ES: milli_per_buffer = %d ms\n",     (int)opaque->milli_per_buffer);
    ALOGI("OpenSL-ES: frame_per_buffer = %d frames\n", (int)opaque->frames_per_buffer);
    ALOGI("OpenSL-ES: bytes_per_buffer = %d bytes\n",  (int)opaque->bytes_per_buffer);
    ALOGI("OpenSL-ES: buffer_capacity  = %d bytes\n",  (int)opaque->buffer_capacity);
    opaque->buffer          = malloc(opaque->buffer_capacity);
    CHECK_COND_ERROR(opaque->buffer, "%s: failed to alloc buffer %d\n", __func__, (int)opaque->buffer_capacity);

    // (*opaque->slPlayItf)->SetPositionUpdatePeriod(opaque->slPlayItf, 1000);

    // enqueue empty buffer to start play
    memset(opaque->buffer, 0, opaque->buffer_capacity);
    for(int i = 0; i < OPENSLES_BUFFERS; ++i) {
        ret = (*opaque->slBufferQueueItf)->Enqueue(opaque->slBufferQueueItf, opaque->buffer + i * opaque->bytes_per_buffer, opaque->bytes_per_buffer);
        CHECK_OPENSL_ERROR(ret, "%s: slBufferQueueItf->Enqueue(000...) failed", __func__);
    }

    opaque->pause_on = 1;
    opaque->abort_request = 0;
    opaque->audio_tid = SDL_CreateThreadEx(&opaque->_audio_tid, aout_thread, aout, "ff_aout_opensles");
    CHECK_COND_ERROR(opaque->audio_tid, "%s: failed to SDL_CreateThreadEx", __func__);

    if (obtained) {
        *obtained      = *desired;
        obtained->size = opaque->buffer_capacity;
        obtained->freq = format_pcm->samplesPerSec / 1000;
    }

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

static void aout_set_volume(SDL_Aout *aout, float left_volume, float right_volume)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    SDL_LockMutex(opaque->wakeup_mutex);
    ALOGI("aout_set_volume(%f, %f)", left_volume, right_volume);
    opaque->left_volume = left_volume;
    opaque->right_volume = right_volume;
    opaque->need_set_volume = 1;
    SDL_CondSignal(opaque->wakeup_cond);
    SDL_UnlockMutex(opaque->wakeup_mutex);
}

static double aout_get_latency_seconds(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;

    SLAndroidSimpleBufferQueueState state = {0};
    SLresult slRet = (*opaque->slBufferQueueItf)->GetState(opaque->slBufferQueueItf, &state);
    if (slRet != SL_RESULT_SUCCESS) {
        ALOGE("%s failed\n", __func__);
        return ((double)opaque->milli_per_buffer) * OPENSLES_BUFFERS / 1000;
    }

    // assume there is always a buffer in coping
    double latency = ((double)opaque->milli_per_buffer) * state.count / 1000;
    return latency;
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

    aout->free_l       = aout_free_l;
    aout->opaque_class = &g_opensles_class;
    aout->open_audio   = aout_open_audio;
    aout->pause_audio  = aout_pause_audio;
    aout->flush_audio  = aout_flush_audio;
    aout->close_audio  = aout_close_audio;
    aout->set_volume   = aout_set_volume;
    aout->func_get_latency_seconds = aout_get_latency_seconds;

    return aout;
fail:
    aout_free_l(aout);
    return NULL;
}

bool SDL_AoutAndroid_IsObjectOfOpenSLES(SDL_Aout *aout)
{
    if (aout)
        return false;

    return aout->opaque_class == &g_opensles_class;
}
