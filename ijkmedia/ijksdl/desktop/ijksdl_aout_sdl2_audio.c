/*****************************************************************************
* ijksdl_vout_callback.c
*****************************************************************************
*
* copyright (c) 2020 befovy <befovy@gmail.com>
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
#include "ijksdl_aout_sdl2_audio.h"
#include "../ijksdl_aout_internal.h"
#include "../ijksdl_stdinc.h"
#include "../ijksdl_log.h"
#include <SDL_audio.h>
#include <SDL.h>
#include <assert.h>


static SDL_Class audio_sdl2_class = {.name = "SDL2Audio"};

typedef struct SDL_Aout_Opaque {
    SDL_AudioDeviceID devid;
} SDL_Aout_Opaque;

static int aout_open_audio_l(SDL_Aout *aout, const SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    assert(aout);

    SDL_Aout_Opaque *opaque = aout->opaque;
    opaque->devid = SDL_OpenAudioDevice(NULL, 0, desired, obtained, 0);
    if (opaque->devid == 0) {
        ALOGE("Failed to open audio: %s", SDL_GetError());
        return -1;
    } else {
        if (desired->format != obtained->format) {
            ALOGW("We didn't get Float32 audio format.");
        }
        return 0;
    }
}
static void aout_close_audio_l(SDL_Aout *aout)
{
    if (!aout || !aout->opaque)
        return;
    SDL_Aout_Opaque *opaque = aout->opaque;
    if (opaque->devid > 0)
        SDL_CloseAudioDevice(opaque->devid);
    opaque->devid = 0;
}

static void aout_pause_audio_l(SDL_Aout *aout, int pause_on)
{
    if (!aout || !aout->opaque)
        return;
    SDL_Aout_Opaque *opaque = aout->opaque;
    if (opaque->devid > 0)
        SDL_PauseAudioDevice(opaque->devid, pause_on);
    ALOGI("aout_uni, aout_pause_audio, aout(%p) pause(%d)\n", aout, pause_on);
}

static void aout_free_l(SDL_Aout *aout) {
    if (!aout)
        return;
    aout_close_audio_l(aout);
    SDL_Aout_FreeInternal(aout);
    ALOGI("aout_uni, aout_free_l, aout(%p)\n", aout);
}

SDL_Aout *SDL_Aout_SDL2_Audio_Create() {

    SDL_Aout *aout = SDL_Aout_CreateInternal(sizeof(SDL_Aout_Opaque));
    if (!aout)
        return NULL;

    aout->opaque_class = &audio_sdl2_class;
    aout->free_l = aout_free_l;
    aout->open_audio = aout_open_audio_l;
    aout->pause_audio = aout_pause_audio_l;
    aout->close_audio = aout_close_audio_l;

    return aout;
}