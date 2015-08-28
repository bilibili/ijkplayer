/*
 * opt.h
 *
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

#ifndef FFPLAY__IJKOPT_H
#define FFPLAY__IJKOPT_H

#include "libavutil/opt.h"

#define IJKAV_OPTION_INT(default__, min__, max__) \
    .type = AV_OPT_TYPE_INT, \
    { .i64 = default__ }, \
    .min = min__, \
    .max = max__, \
    .flags = AV_OPT_FLAG_DECODING_PARAM

#define IJKAV_OPTION_INT64(default__, min__, max__) \
    .type = AV_OPT_TYPE_INT64, \
    { .i64 = default__ }, \
    .min = min__, \
    .max = max__, \
    .flags = AV_OPT_FLAG_DECODING_PARAM

#define IJKAV_OPTION_CONST(default__) \
    .type = AV_OPT_TYPE_CONST, \
    { .i64 = default__ }, \
    .min = INT_MIN, \
    .max = INT_MAX, \
    .flags = AV_OPT_FLAG_DECODING_PARAM

#define IJKAV_OPTION_STR(default__) \
    .type = AV_OPT_TYPE_STRING, \
    { .str = default__ }, \
    .min = 0, \
    .max = 0, \
    .flags = AV_OPT_FLAG_DECODING_PARAM

#endif
