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
    {                                                                   \
        extern URLProtocol ff_##x##_protocol;                           \
        ijkav_register_protocol(&ff_##x##_protocol);                    \
    }

#define IJK_REGISTER_PROTOCOL(x)                                        \
    {                                                                   \
        extern URLProtocol ijkff_##x##_protocol;                        \
        ijkav_register_protocol(&ijkff_##x##_protocol);                 \
    }

#define IJK_REGISTER_DEMUXER(x)                                         \
    {                                                                   \
        extern AVInputFormat ijkff_##x##_demuxer;                       \
        ijkav_register_input_format(&ijkff_##x##_demuxer);              \
    }


static struct URLProtocol *ijkav_find_protocol(const char *proto_name)
{
    URLProtocol *up = NULL;
    if (!proto_name)
        return NULL;
    while ((up = ffurl_protocol_next(up)) != NULL) {
        if (!up->name)
            continue;
        if (!strcmp(proto_name, up->name))
            break;
    }
    return up;
}

static struct AVInputFormat *ijkav_find_input_format(const char *iformat_name)
{
    AVInputFormat *fmt = NULL;
    if (!iformat_name)
        return NULL;
    while ((fmt = av_iformat_next(fmt))) {
        if (!fmt->name)
            continue;
        if (!strcmp(iformat_name, fmt->name))
            return fmt;
    }
    return NULL;
}

static void ijkav_register_protocol(URLProtocol *protocol)
{
    if (ijkav_find_protocol(protocol->name)) {
        av_log(NULL, AV_LOG_WARNING, "skip     protocol: %s (duplicated)\n", protocol->name);
    } else {
        av_log(NULL, AV_LOG_INFO,    "register protocol: %s\n", protocol->name);
        ffurl_register_protocol(protocol);
    }
}

static void ijkav_register_input_format(AVInputFormat *iformat)
{
    if (ijkav_find_input_format(iformat->name)) {
        av_log(NULL, AV_LOG_WARNING, "skip     demuxer : %s (duplicated)\n", iformat->name);
    } else {
        av_log(NULL, AV_LOG_INFO,    "register demuxer : %s\n", iformat->name);
        av_register_input_format(iformat);
    }
}

void ijkav_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;

    av_register_all();

    /* protocols */
    av_log(NULL, AV_LOG_INFO, "===== custom modules begin =====\n");
    FF_REGISTER_PROTOCOL(async);
    IJK_REGISTER_PROTOCOL(ijkhttphook);
    IJK_REGISTER_PROTOCOL(ijkinject);
    IJK_REGISTER_PROTOCOL(ijklongurl);
#ifdef __ANDROID__
    IJK_REGISTER_PROTOCOL(ijkmediadatasource);
#endif
    IJK_REGISTER_PROTOCOL(ijksegment);
    IJK_REGISTER_PROTOCOL(ijktcphook);

    /* demuxers */
    IJK_REGISTER_DEMUXER(ijklivehook);
    av_log(NULL, AV_LOG_INFO, "===== custom modules end =====\n");
}
