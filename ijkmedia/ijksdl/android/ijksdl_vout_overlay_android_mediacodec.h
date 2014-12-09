/*****************************************************************************
 * ijksdl_vout_overlay_android_mediacodec.h
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

#ifndef IJKSDL_ANDROID__IJKSDL_VOUT_OVERLAY_ANDROID_MEDIACODEC_H
#define IJKSDL_ANDROID__IJKSDL_VOUT_OVERLAY_ANDROID_MEDIACODEC_H

#include "../ijksdl_stdinc.h"
#include "../ijksdl_vout.h"

typedef struct SDL_AMediaCodec           SDL_AMediaCodec;
typedef struct SDL_AMediaCodecBufferInfo SDL_AMediaCodecBufferInfo;

SDL_VoutOverlay *SDL_VoutAMediaCodec_CreateOverlay(int width, int height, Uint32 format, SDL_Vout *vout);

bool SDL_VoutOverlayAMediaCodec_isKindOf(SDL_VoutOverlay *overlay);
int  SDL_VoutOverlayAMediaCodec_attachFrame(
     SDL_VoutOverlay *overlay,
     SDL_AMediaCodec *acodec,
     int output_buffer_index,
     SDL_AMediaCodecBufferInfo *buffer_info);
int  SDL_VoutOverlayAMediaCodec_releaseFrame(SDL_VoutOverlay *overlay, SDL_AMediaCodec *acodec, bool render);

#endif
