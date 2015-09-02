/*
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifndef FFPLAY__VERSION_H
#define FFPLAY__VERSION_H

#include "libavutil/version.h"

/*
 * The major version number is incremented with backward incompatible changes
 *   - e.g. removing parts of the public API, reordering public struct members, etc. 
 * The minor version number is incremented for backward compatible API changes
 * or major new features
 *   - e.g. adding a new public function or a new decoder.
 * The micro version number is incremented for smaller changes that a calling program
 * might still want to check for
 *   - e.g. changing behavior in a previously unspecified situation.
 */

#define LIBIJKPLAYER_VERSION_MAJOR  2
#define LIBIJKPLAYER_VERSION_MINOR  0
#define LIBIJKPLAYER_VERSION_MICRO  0

#define LIBIJKPLAYER_VERSION_INT    AV_VERSION_INT(LIBIJKPLAYER_VERSION_MAJOR, \
                                                   LIBIJKPLAYER_VERSION_MINOR, \
                                                   LIBIJKPLAYER_VERSION_MICRO)
#define LIBIJKPLAYER_VERSION        AV_VERSION(LIBIJKPLAYER_VERSION_MAJOR,     \
                                               LIBIJKPLAYER_VERSION_MINOR,     \
                                               LIBIJKPLAYER_VERSION_MICRO)
#define LIBIJKPLAYER_BUILD          LIBIJKPLAYER_VERSION_INT

#define LIBIJKPLAYER_IDENT          "ijkplayer " AV_STRINGIFY(LIBIJKPLAYER_VERSION)

#define IJKVERSION_GET_MAJOR(x)     ((x >> 16) & 0xFF)
#define IJKVERSION_GET_MINOR(x)     ((x >>  8) & 0xFF)
#define IJKVERSION_GET_MICRO(x)     ((x      ) & 0xFF)

#endif//FFPLAY__VERSION_H
