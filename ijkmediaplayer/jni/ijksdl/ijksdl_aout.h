/*****************************************************************************
 * ijksdl_aout.h
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

#ifndef IJKSDL__IJKSDL_AOUT_H
#define IJKSDL__IJKSDL_AOUT_H

#include "ijksdl_audio.h"
#include "ijksdl_mutex.h"

typedef struct SDL_Aout_Opaque SDL_Aout_Opaque;
typedef struct SDL_Aout SDL_Aout;
typedef struct SDL_Aout {
    SDL_mutex *mutex;

    SDL_Aout_Opaque *opaque;
    void (*free_l)(SDL_Aout *vout);
    int (*open_audio)(SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
    void (*play_audio)(SDL_Aout *aout);
    void (*pause_audio)(SDL_Aout *aout);
    void (*close_audio)(SDL_Aout *aout);
} SDL_Aout;

int SDL_AoutOpenAudio(SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained);
void SDL_AoutPlayAudio(SDL_Aout *aout);
void SDL_AoutPauseAudio(SDL_Aout *aout);
void SDL_AoutCloseAudio(SDL_Aout *aout);
void SDL_AoutFreeAudio(SDL_Aout *aout);

#endif
