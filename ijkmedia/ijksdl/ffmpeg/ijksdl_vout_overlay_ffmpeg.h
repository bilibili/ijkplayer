/*****************************************************************************
 * ijksdl_vout_overlay_ffmpeg.h
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

#ifndef IJKSDL__FFMPEG__IJKSDL_VOUT_OVERLAY_FFMPEG_H
#define IJKSDL__FFMPEG__IJKSDL_VOUT_OVERLAY_FFMPEG_H

#include "../ijksdl_stdinc.h"
#include "../ijksdl_vout.h"
#include "ijksdl_inc_ffmpeg.h"

// TODO: 9 alignment to speed up memcpy when display
SDL_VoutOverlay *SDL_VoutFFmpeg_CreateOverlay(int width, int height, int frame_format, SDL_Vout *vout);

#endif
