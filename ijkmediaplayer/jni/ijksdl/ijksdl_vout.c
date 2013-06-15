/*****************************************************************************
 * ijksdl_vout.c
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

#include "ijksdl_vout.h"

#include <assert.h>
#include <android/native_window_jni.h>

void SDL_VoutFree(SDL_Vout *vout)
{
    if (!vout)
        return;

    if (vout->free_l) {
        vout->free_l(vout);
    } else {
        free(vout);
    }
}

int SDL_VoutGetSurface(SDL_Vout *vout, SDL_VoutSurface** ppsurface, int w, int h, int format)
{
    assert(ppsurface);
    if (!ppsurface || !vout || !vout->get_surface)
        return -1;

    SDL_LockMutex(vout->mutex);
    int retval = vout->get_surface(vout, ppsurface, w, h, format);
    SDL_UnlockMutex(vout->mutex);

    return retval;
}

void SDL_VoutSurface_Free(SDL_VoutSurface *surface)
{
    if (!surface)
        return;

    if (surface->free_l)
        surface->free_l(surface);
    else
        free(surface);
}

