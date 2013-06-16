/*****************************************************************************
 * ijksdl_vout_internal.h
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

#ifndef IJKSDL__IJKSDL_VOUT_INTERNAL_H
#define IJKSDL__IJKSDL_VOUT_INTERNAL_H

#include "ijksdl_vout.h"

inline static SDL_Vout *SDL_Vout_CreateInternal()
{
    SDL_Vout *vout = (SDL_Vout*) malloc(sizeof(SDL_Vout));
    if (!vout)
        return NULL;

    memset(vout, 0, sizeof(SDL_Vout));
    vout->mutex = SDL_CreateMutex();
    if (vout->mutex == NULL) {
        free(vout);
        return NULL;
    }

    return vout;
}

inline static void SDL_Vout_FreeInternal(SDL_Vout *vout)
{
    if (!vout)
        return;

    if (vout->mutex) {
        SDL_DestroyMutex(vout->mutex);
    }

    memset(vout, 0, sizeof(SDL_VoutSurface));
    free(vout);
}

inline static SDL_VoutSurface *SDL_Vout_CreateSurfaceInternal()
{
    SDL_VoutSurface *surface = (SDL_VoutSurface*) malloc(sizeof(SDL_VoutSurface));
    if (!surface)
        return NULL;

    memset(surface, 0, sizeof(SDL_VoutSurface));
    return surface;
}

inline static void SDL_Vout_FreeSurfaceInternal(SDL_VoutSurface *surface)
{
    if (!surface)
        return;

    memset(surface, 0, sizeof(SDL_VoutSurface));
    free(surface);
}

#endif
