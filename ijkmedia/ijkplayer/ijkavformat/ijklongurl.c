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

#include <assert.h>
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"

#include "ijkplayer/ijkavutil/opt.h"
#include "ijkavformat.h"

typedef struct Context {
    AVClass        *class;
    URLContext     *inner;

    /* options */
    char           *url;
} Context;

static int ijklongurl_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;

    if (!c->url || !*c->url)
        return AVERROR_EXTERNAL;

    return ffurl_open(&c->inner, c->url, flags, &h->interrupt_callback, options);
}

static int ijklongurl_close(URLContext *h)
{
    Context *c = h->priv_data;

    return ffurl_close(c->inner);
}

static int ijklongurl_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;

    return ffurl_read(c->inner, buf, size);
}

static int64_t ijklongurl_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;

    return ffurl_seek(c->inner, pos, whence);
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijklongurl-url",         "real url to access",
        OFFSET(url),            IJKAV_OPTION_STR(NULL) },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijklongurl_context_class = {
    .class_name = "LongUrl",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkff_ijklongurl_protocol = {
    .name                = "ijklongurl",
    .url_open2           = ijklongurl_open,
    .url_read            = ijklongurl_read,
    .url_seek            = ijklongurl_seek,
    .url_close           = ijklongurl_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijklongurl_context_class,
};
