/*****************************************************************************
* ijksdl_aout_port_audio.c
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
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

#include "ijksdl_aout_port_audio.h"

#include "../ijksdl_misc.h"
#include "../ijksdl_stdinc.h"
#include "../ijksdl_log.h"
#include "../ijksdl_aout_internal.h"

#include <portaudio.h>
#include <assert.h>

static SDL_Class audio_port_class = { .name = "PortAudio" };

typedef struct SDL_Aout_Opaque
{
    PaStreamParameters outputParameters;
    PaStream *stream;
    int x;

    int channels;
    void *userdata;
    SDL_AudioCallback callback;
} SDL_Aout_Opaque;


int portAudioStreamCallback(const void *input, void *output, unsigned long frameCount, 
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData)
{
    (void) input;
    (void) timeInfo;
    (void) statusFlags;

    SDL_Aout_Opaque *opaque = userData;
    opaque->callback(opaque->userdata, output, frameCount * 2 * opaque->channels);
  
    return paContinue;
}
static int aout_open_audio_l(SDL_Aout *aout, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    assert(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;

    opaque->outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (opaque->outputParameters.device == paNoDevice) {
        ALOGE("PortAudio Error: No default output device.\n");
        return -1;
    }

    PaError err;
    opaque->outputParameters.channelCount = desired->channels;       /* stereo output */
    opaque->outputParameters.sampleFormat = paInt16; /* 32 bit floating point output */
    opaque->outputParameters.suggestedLatency = Pa_GetDeviceInfo(opaque->outputParameters.device )->defaultLowOutputLatency;
    opaque->outputParameters.hostApiSpecificStreamInfo = NULL;


    opaque->channels = desired->channels;
    opaque->userdata = desired->userdata;
    opaque->callback = desired->callback;

    if (obtained) {
        memcpy(obtained, desired, sizeof(SDL_AudioSpec));
        obtained->size = 1024;
        obtained->freq = desired->freq;
        obtained->format = AUDIO_S16SYS;
    }

    err = Pa_OpenStream(
        &opaque->stream,
        NULL, /* no input */
        &opaque->outputParameters,
        desired->freq,
        128,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        portAudioStreamCallback, /* no callback, use blocking API */
        opaque);

    return 0;
}

static void aout_close_audio_l(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    
    if (opaque->stream) {
        Pa_CloseStream(opaque->stream);
        opaque->stream = NULL;
    }
}

static void aout_free_l(SDL_Aout *aout)
{
    if (!aout)
        return;
    aout_close_audio_l(aout);
    SDL_Aout_FreeInternal(aout);
    ALOGI("aout_uni, aout_free_l, aout(%p)\n", aout);
}

static int aout_pause_audio_l(SDL_Aout *aout, int pause_on)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
    if (opaque->stream){
        if (pause_on)
            Pa_StopStream(opaque->stream);
        else 
            Pa_StartStream(opaque->stream);
    }
    ALOGI("aout_uni, aout_pause_audio, aout(%p) pause(%d)\n", aout, pause_on);
    return 0;
}

static void aout_flush_audio_l(SDL_Aout *aout)
{
    SDL_Aout_Opaque *opaque = aout->opaque;
}

static double aout_get_latency_seconds_l(SDL_Aout *aout)
{
    // SDL_Aout_Opaque *opaque = aout->opaque;
    return 0.0;
}


SDL_Aout *SDL_Aout_Port_Audio_Create()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
        return NULL;
    SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;    

    aout->opaque_class = &audio_port_class;
    aout->free_l = aout_free_l;
    aout->open_audio = aout_open_audio_l;
    aout->pause_audio = aout_pause_audio_l;
    aout->close_audio = aout_close_audio_l;

    // aout->func_get_latency_seconds = aout_get_latency_seconds_l;
    // aout->flush_audio = aout_flush_audio_l;

    return aout;
}
