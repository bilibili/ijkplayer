/*****************************************************************************
 * ijksdl_video.h
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

#ifndef IJKSDL__IJKSDL_VIDEO_H
#define IJKSDL__IJKSDL_VIDEO_H

#include "ijksdl_stdinc.h"

/** @name SDL_Surface Flags
 *  These are the currently supported flags for the SDL_surface
 */
/*@{*/

/** Available for SDL_CreateRGBSurface() or SDL_SetVideoMode() */
/*@{*/
#define SDL_SWSURFACE   0x00000000  /**< Surface is in system memory */
#define SDL_HWSURFACE   0x00000001  /**< Surface is in video memory */
#define SDL_ASYNCBLIT   0x00000004  /**< Use asynchronous blits if possible */
/*@}*/

/** Available for SDL_SetVideoMode() */
/*@{*/
#define SDL_ANYFORMAT   0x10000000  /**< Allow any video depth/pixel-format */
#define SDL_HWPALETTE   0x20000000  /**< Surface has exclusive palette */
#define SDL_DOUBLEBUF   0x40000000  /**< Set up double-buffered video mode */
#define SDL_FULLSCREEN  0x80000000  /**< Surface is a full screen display */
#define SDL_OPENGL      0x00000002      /**< Create an OpenGL rendering context */
#define SDL_OPENGLBLIT  0x0000000A  /**< Create an OpenGL rendering context and use it for blitting */
#define SDL_RESIZABLE   0x00000010  /**< This video mode may be resized */
#define SDL_NOFRAME 0x00000020  /**< No window caption or edge frame */
/*@}*/

/** Used internally (read-only) */
/*@{*/
#define SDL_HWACCEL 0x00000100  /**< Blit uses hardware acceleration */
#define SDL_SRCCOLORKEY 0x00001000  /**< Blit uses a source color key */
#define SDL_RLEACCELOK  0x00002000  /**< Private flag */
#define SDL_RLEACCEL    0x00004000  /**< Surface is RLE encoded */
#define SDL_SRCALPHA    0x00010000  /**< Blit uses source alpha blending */
#define SDL_PREALLOC    0x01000000  /**< Surface uses preallocated memory */
/*@}*/

/*@}*/

/*-
 *  http://www.webartz.com/fourcc/indexyuv.htm
 *  http://www.neuro.sfc.keio.ac.jp/~aly/polygon/info/color-space-faq.html
 *  http://www.fourcc.org/yuv.php
 */
// bpp=12
#define SDL_YV12_OVERLAY  0x32315659    /**< Planar mode: Y + V + U  (3 planes) */
#define SDL_IYUV_OVERLAY  0x56555949    /**< Planar mode: Y + U + V  (3 planes) */
// bpp=16
#define SDL_YUY2_OVERLAY  0x32595559    /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
#define SDL_UYVY_OVERLAY  0x59565955    /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
#define SDL_YVYU_OVERLAY  0x55595659    /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */

#endif
