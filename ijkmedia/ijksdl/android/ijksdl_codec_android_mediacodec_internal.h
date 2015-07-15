/*****************************************************************************
 * ijksdl_codec_android_mediacodec_internal.h
 *****************************************************************************
 *
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_INTERNAL_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIACODEC_INTERNAL_H

#include "ijksdl_codec_android_mediacodec.h"

inline static SDL_AMediaCodec *SDL_AMediaCodec_CreateInternal(size_t opaque_size)
{
    SDL_AMediaCodec *acodec = (SDL_AMediaCodec*) mallocz(sizeof(SDL_AMediaCodec));
    if (!acodec)
        return NULL;

    acodec->opaque = mallocz(opaque_size);
    if (!acodec->opaque) {
        free(acodec);
        return NULL;
    }

    acodec->mutex = SDL_CreateMutex();
    if (acodec->mutex == NULL) {
        free(acodec->opaque);
        free(acodec);
        return NULL;
    }

    return acodec;
}

inline static void SDL_AMediaCodec_FreeInternal(SDL_AMediaCodec *acodec)
{
    if (!acodec)
        return;

    if (acodec->mutex) {
        SDL_DestroyMutexP(&acodec->mutex);
    }

    free(acodec->opaque);
    memset(acodec, 0, sizeof(SDL_AMediaCodec));
    free(acodec);
}

#endif

