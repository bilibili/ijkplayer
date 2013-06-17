/*****************************************************************************
 * ijksdl_aout_internal.h
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

#ifndef IJKSDL__IJKSDL_AOUT_INTERNAL_H
#define IJKSDL__IJKSDL_AOUT_INTERNAL_H

#include "ijksdl_mutex.h"
#include "ijksdl_aout.h"

inline static SDL_Aout *SDL_Aout_CreateInternal()
{
    SDL_Aout *aout = (SDL_Aout*) malloc(sizeof(SDL_Aout));
    if (!aout)
        return NULL;

    memset(aout, 0, sizeof(SDL_Aout));
    aout->mutex = SDL_CreateMutex();
    if (aout->mutex == NULL) {
        free(aout);
        return NULL;
    }

    return aout;
}

inline static void SDL_Aout_FreeInternal(SDL_Aout *aout)
{
    if (!aout)
        return;

    if (aout->mutex) {
        SDL_DestroyMutex(aout->mutex);
    }

    memset(aout, 0, sizeof(SDL_Aout));
    free(aout);
}

#endif
