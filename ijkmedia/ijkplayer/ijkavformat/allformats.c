/*
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
 *
 * This file is part of ijkPlayer.
 * Based on libavformat/allformats.c
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavformat/version.h"

#define FF_REGISTER_PROTOCOL(x)                                         \
{                                                                       \
    extern URLProtocol ff_##x##_protocol;                               \
    ffurl_register_protocol(&ff_##x##_protocol);                        \
}

#define IJK_REGISTER_PROTOCOL(x)                                        \
    {                                                                   \
        extern URLProtocol ijkff_##x##_protocol;                        \
        ffurl_register_protocol(&ijkff_##x##_protocol);                 \
    }

#define IJK_REGISTER_DEMUXER(x)                                         \
    {                                                                   \
        extern AVInputFormat ijkff_##x##_demuxer;                       \
        av_register_input_format(&ijkff_##x##_demuxer);                 \
    }

void ijkav_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;

    /* protocols */
    FF_REGISTER_PROTOCOL(async);
    IJK_REGISTER_PROTOCOL(ijkhttphook);
    IJK_REGISTER_PROTOCOL(ijkinject);
    IJK_REGISTER_PROTOCOL(ijklongurl);
    IJK_REGISTER_PROTOCOL(ijksegment);
    IJK_REGISTER_PROTOCOL(ijktcphook);

    /* demuxers */
    IJK_REGISTER_DEMUXER(ijklivehook);
}
