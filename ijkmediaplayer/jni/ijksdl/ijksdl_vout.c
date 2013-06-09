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

#include "ijksdl_vout.h"

SDL_Vout *SDL_VoutCreate()
{
    SDL_Vout *vout = malloc(sizeof(SDL_Vout));
    memset(vout, 0, sizeof(SDL_Vout));

    /* FIXME: implement */
    return vout;
}

void SDL_VoutFree(SDL_Vout *vout)
{
    if (!vout)
        return;

    /* FIXME: implement */

    free(vout);
}

int SDL_VoutSetBuffersGeometry(SDL_Vout *vout, int32_t width, int32_t height, int32_t format)
{
    /* FIXME: implement */
    return 0;
}

int SDL_VoutRender(SDL_Vout *vout, SDL_Picture *pic)
{
    /* FIXME: implement */
    return 0;
}
