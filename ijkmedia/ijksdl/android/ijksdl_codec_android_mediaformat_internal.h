/*****************************************************************************
 * ijksdl_codec_android_mediaformat_internal.h
 *****************************************************************************
 *
 * Copyright (c) 2014 Bilibili
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIAFORMAT_INTERNAL_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIAFORMAT_INTERNAL_H

#include "ijksdl_codec_android_mediaformat.h"

inline static SDL_AMediaFormat *SDL_AMediaFormat_CreateInternal(size_t opaque_size)
{
    SDL_AMediaFormat *aformat = (SDL_AMediaFormat*) mallocz(sizeof(SDL_AMediaFormat));
    if (!aformat)
        return NULL;

    aformat->opaque = mallocz(opaque_size);
    if (!aformat->opaque) {
        free(aformat);
        return NULL;
    }

    aformat->mutex = SDL_CreateMutex();
    if (aformat->mutex == NULL) {
        free(aformat->opaque);
        free(aformat);
        return NULL;
    }

    return aformat;
}

inline static void SDL_AMediaFormat_FreeInternal(SDL_AMediaFormat *aformat)
{
    if (!aformat)
        return;

    if (aformat->mutex) {
        SDL_DestroyMutexP(&aformat->mutex);
    }

    free(aformat->opaque);
    memset(aformat, 0, sizeof(SDL_AMediaFormat));
    free(aformat);
}

#endif

