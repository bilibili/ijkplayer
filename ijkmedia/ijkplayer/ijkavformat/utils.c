/*
 * utils.c
 *
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include <stdlib.h>
#include "ijkavformat.h"

static IjkAVInjectCallback s_av_inject_callback = NULL;

IjkAVInjectCallback ijkav_register_inject_callback(IjkAVInjectCallback callback)
{
    IjkAVInjectCallback prev_callback = s_av_inject_callback;
    s_av_inject_callback = callback;
    return prev_callback;
}

IjkAVInjectCallback ijkav_get_inject_callback()
{
    return s_av_inject_callback;
}
