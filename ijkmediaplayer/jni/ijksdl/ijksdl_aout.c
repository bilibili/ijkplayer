/*****************************************************************************
 * ijksdl_aout.c
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

#include "ijksdl_aout.h"

int SDL_AoutOpenAudio(SDL_Aout *aout, SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    if (aout && desired && aout->open_audio)
        return aout->open_audio(aout, desired, obtained);

    return -1;
}

void SDL_AoutPlayAudio(SDL_Aout *aout)
{
    if (aout && aout->play_audio)
        return aout->play_audio(aout);
}

void SDL_AoutPauseAudio(SDL_Aout *aout)
{
    if (aout && aout->pause_audio)
        return aout->pause_audio(aout);
}

void SDL_AoutCloseAudio(SDL_Aout *aout)
{
    if (aout && aout->close_audio)
        return aout->close_audio(aout);
}

void SDL_AoutFreeAudio(SDL_Aout *aout)
{
    if (aout && aout->free_l)
        return aout->free_l(aout);
}
