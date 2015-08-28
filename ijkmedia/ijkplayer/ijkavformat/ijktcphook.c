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
    int64_t         opaque;
    int             segment_index;
    int64_t         test_fail_point;
    int64_t         read_bytes;
} Context;

static void *ijkinject_get_opaque(URLContext *h) {
    Context *c = h->priv_data;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
    return (void *)c->opaque;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}

static int ijktcphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    IJKAVInject_OnUrlOpenData inject_data = {0};
    IjkAVInjectCallback inject_callback = ijkav_get_inject_callback();
    void *opaque = ijkinject_get_opaque(h);
    int ret = 0;

    av_strstart(arg, "ijktcphook:", &arg);

    inject_data.size = sizeof(inject_data);
    inject_data.segment_index = c->segment_index;
    snprintf(inject_data.url, sizeof(inject_data.url), "tcp:%s", arg);

    if (opaque && inject_callback) {
        av_log(h, AV_LOG_INFO, "tcp-hook %s\n", inject_data.url);
        ret = inject_callback(opaque, IJKAVINJECT_ON_TCP_OPEN, &inject_data, sizeof(inject_data));
        if (ret || !inject_data.url[0]) {
            ret = AVERROR_EXIT;
            goto fail;
        }
    }

    av_dict_set_int(options, "ijkinject-opaque",        c->opaque, 0);
    av_dict_set_int(options, "ijkinject-segment-index", c->segment_index, 0);
    ret = ffurl_open(&c->inner, inject_data.url, flags, &h->interrupt_callback, options);
    if (ret)
        goto fail;

fail:
    return ret;
}

static int ijktcphook_close(URLContext *h)
{
    Context *c = h->priv_data;

    return ffurl_close(c->inner);
}

static int ijktcphook_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    int ret = ffurl_read(c->inner, buf, size);

#ifdef DEBUG
    if (ret > 0) {
        c->read_bytes += ret;
        if (c->test_fail_point > 0 && c->read_bytes >= c->test_fail_point)
            return AVERROR(EIO);
    }
#endif

    return ret;
}

static int ijktcphook_write(URLContext *h, const unsigned char *buf, int size)
{
    Context *c = h->priv_data;

    return ffurl_write(c->inner, buf, size);
}

static int64_t ijktcphook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;

    return ffurl_seek(c->inner, pos, whence);
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijkinject-opaque",           "private data of user, passed with custom callback",
        OFFSET(opaque),             IJKAV_OPTION_INT64(0, INT64_MIN, INT64_MAX) },
    { "ijkinject-segment-index",    "segment index of current url",
        OFFSET(segment_index),      IJKAV_OPTION_INT(0, 0, INT_MAX) },
    { "ijktcphook-test-fail-point", "test fail point, in bytes",
        OFFSET(test_fail_point),    IJKAV_OPTION_INT(0, 0, INT_MAX) },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijktcphook_context_class = {
    .class_name = "TcpHook",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkff_ijktcphook_protocol = {
    .name                = "ijktcphook",
    .url_open2           = ijktcphook_open,
    .url_read            = ijktcphook_read,
    .url_write           = ijktcphook_write,
    .url_seek            = ijktcphook_seek,
    .url_close           = ijktcphook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijktcphook_context_class,
};
