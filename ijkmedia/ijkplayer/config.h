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

#ifndef FFPLAY__CONFIG_H
#define FFPLAY__CONFIG_H

#include "libffmpeg/config.h"

// FIXME: merge filter related code and enable it
#ifdef CONFIG_AVFILTER
// #undef CONFIG_AVFILTER
#endif
// #define CONFIG_AVFILTER 0

#ifdef FFP_MERGE
#undef FFP_MERGE
#endif

#ifdef FFP_SUB
#undef FFP_SUB
#endif

#ifndef FFMPEG_LOG_TAG
#define FFMPEG_LOG_TAG "IJKFFMPEG"
#endif

#endif//FFPLAY__CONFIG_H
