/*****************************************************************************
 * ijksdl_vout_internal.h
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

#ifndef IJKSDL__IJKSDL_VOUT_INTERNAL_H
#define IJKSDL__IJKSDL_VOUT_INTERNAL_H

#include <stdlib.h>
#include <string.h>
#include "ijksdl_vout.h"

inline static SDL_Vout *SDL_Vout_CreateInternal(size_t opaque_size)
{
    SDL_Vout *vout = (SDL_Vout*) calloc(1, sizeof(SDL_Vout));
    if (!vout)
        return NULL;

    vout->opaque = calloc(1, opaque_size);
    if (!vout->opaque) {
        free(vout);
        return NULL;
    }

    vout->mutex = SDL_CreateMutex();
    if (vout->mutex == NULL) {
        free(vout->opaque);
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

    free(vout->opaque);
    memset(vout, 0, sizeof(SDL_Vout));
    free(vout);
}

inline static SDL_VoutOverlay *SDL_VoutOverlay_CreateInternal(size_t opaque_size)
{
    SDL_VoutOverlay *overlay = (SDL_VoutOverlay*) calloc(1, sizeof(SDL_VoutOverlay));
    if (!overlay)
        return NULL;

    overlay->opaque = calloc(1, opaque_size);
    if (!overlay->opaque) {
        free(overlay);
        return NULL;
    }
    return overlay;
}

inline static void SDL_VoutOverlay_FreeInternal(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return;

    if (overlay->opaque)
        free(overlay->opaque);

    memset(overlay, 0, sizeof(SDL_VoutOverlay));
    free(overlay);
}

#endif
