/*****************************************************************************
 * ijksdl_video.c
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

#include "ijksdl_video.h"

void SDL_FreeSurface(SDL_Surface *surface)
{
    // FIXME: implement
}

SDL_Overlay *SDL_CreateYUVOverlay(int width, int height,
        Uint32 format, SDL_Surface *display)
{
    // FIXME: implement
    return NULL;
}

SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    // FIXME: implement
    return NULL;
}

int SDL_LockYUVOverlay(SDL_Overlay *overlay)
{
    // FIXME: implement
    return 0;
}

void SDL_UnlockYUVOverlay(SDL_Overlay *overlay)
{
    // FIXME: implement
}

int SDL_DisplayYUVOverlay(SDL_Overlay *overlay, SDL_Rect *dstrect)
{
    // FIXME: implement
    return 0;
}

void SDL_FreeYUVOverlay(SDL_Overlay *overlay)
{
    // FIXME: implement
}
