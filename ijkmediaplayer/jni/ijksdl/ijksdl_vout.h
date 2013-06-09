/*****************************************************************************
 * ijksdl_vout.h
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

#ifndef IJKSDL__IJKSDL_VOUT_H
#define IJKSDL__IJKSDL_VOUT_H

#include "ijksdl_stdinc.h"

typedef struct SDL_Picture {
    Uint32    format;
    int       w, h;
    int       planes;
    Uint16   *pitches;
    Uint8   **pixels;
} SDL_Picture;

typedef struct SDL_Vout
{
    int32_t width;
    int32_t height;
    int32_t format;
} SDL_Vout;

SDL_Vout *SDL_VoutCreate();
void      SDL_VoutFreeVout(SDL_Vout *vout);
int       SDL_VoutSetBuffersGeometry(SDL_Vout *vout, int32_t width, int32_t height, int32_t format);
int       SDL_VoutRender(SDL_Vout *vout, SDL_Picture *pic);

#endif
