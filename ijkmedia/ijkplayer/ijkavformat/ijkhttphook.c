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

    int64_t         logic_pos;
    int64_t         logic_size;
    int             seekable;

    /* options */
    int             open_flags;
    AVDictionary   *open_opts;
    int64_t         opaque;
    int             segment_index;
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

static int ijkhttphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    IJKAVInject_OnUrlOpenData inject_data = {0};
    IjkAVInjectCallback inject_callback = ijkav_get_inject_callback();
    void *opaque = ijkinject_get_opaque(h);
    int ret = 0;

    c->open_flags = flags;
    if (options)
        av_dict_copy(&c->open_opts, *options, 0);

    av_strstart(arg, "ijkhttphook:", &arg);

    inject_data.size = sizeof(inject_data);
    inject_data.segment_index = c->segment_index;
    snprintf(inject_data.url, sizeof(inject_data.url), "http:%s", arg);
    if (opaque && inject_callback) {
        av_log(h, AV_LOG_INFO, "http-hook %s\n", inject_data.url);
        ret = inject_callback(opaque, IJKAVINJECT_ON_HTTP_OPEN, &inject_data, sizeof(inject_data));
        if (ret)
            goto fail;
    }

    av_dict_set_int(options, "ijkinject-opaque",        c->opaque, 0);
    av_dict_set_int(options, "ijkinject-segment-index", c->segment_index, 0);
    ret = ffurl_open(&c->inner, inject_data.url, flags, &h->interrupt_callback, options);
    if (ret)
        goto fail;

    c->logic_size = ffurl_size(c->inner);
    c->seekable   = c->logic_size > 0 ? 1 : 0;
fail:
    av_dict_free(&c->open_opts);
    return ret;
}

static int ijkhttphook_close(URLContext *h)
{
    Context *c = h->priv_data;

    av_dict_free(&c->open_opts);

    return ffurl_close(c->inner);
}

static int ijkhttphook_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    void *opaque = ijkinject_get_opaque(h);
    IjkAVInjectCallback inject_callback = ijkav_get_inject_callback();

    int read_ret = ffurl_read(c->inner, buf, size);
    if (opaque && inject_callback && c->seekable) {
        IJKAVInject_OnUrlOpenData inject_data = {0};
        inject_data.size = sizeof(inject_data);
        inject_data.segment_index = c->segment_index;
        while (read_ret < 0) {
            AVDictionary *tmp_opts = NULL;

            inject_data.retry_counter++;

            switch (read_ret) {
                case AVERROR_EOF:
                case AVERROR_EXIT:
                    goto fail;
                case AVERROR(EAGAIN):
                    goto continue_read;
            }

            if (ff_check_interrupt(&h->interrupt_callback)) {
                read_ret = AVERROR_EXIT;
                goto fail;
            }

            av_log(h, AV_LOG_INFO, "http-hook-retry %s (%d)\n", inject_data.url, inject_data.retry_counter);
            int ret = inject_callback(opaque, IJKAVINJECT_ON_HTTP_RETRY, &inject_data, sizeof(inject_data));
            if (ret || !inject_data.url[0]) {
                ret = AVERROR_EXIT;
                goto fail;
            }

            if (c->open_opts)
                av_dict_copy(&tmp_opts, c->open_opts, 0);

            av_dict_set_int(&tmp_opts, "ijkinject-opaque",        c->opaque, 0);
            av_dict_set_int(&tmp_opts, "ijkinject-segment-index", c->segment_index, 0);
            ret = ffurl_open(&c->inner, inject_data.url, c->open_flags, &h->interrupt_callback, &tmp_opts);
            av_dict_free(&tmp_opts);
            if (ret)
                goto continue_retry;

            c->logic_size = ffurl_size(c->inner);
            c->seekable   = c->logic_size > 0 ? 1 : 0;

continue_read:
            read_ret = ffurl_read(c->inner, buf, size);

continue_retry:
            inject_data.retry_counter++;
        }
    }

    if (read_ret > 0)
        c->logic_pos += read_ret;

fail:
    return read_ret;
}

static int64_t ijkhttphook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;

    int64_t seek_ret = ffurl_seek(c->inner, pos, whence);
    if (seek_ret >= 0)
        c->logic_pos = seek_ret;

    return seek_ret;
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijkinject-opaque",       "private data of user, passed with custom callback",
        OFFSET(opaque),         IJKAV_OPTION_INT64(0, INT64_MIN, INT64_MAX) },
    { "ijkinject-segment-index",    "segment index of current url",
        OFFSET(segment_index),      IJKAV_OPTION_INT(0, 0, INT_MAX) },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijkhttphook_context_class = {
    .class_name = "HttpHook",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkff_ijkhttphook_protocol = {
    .name                = "ijkhttphook",
    .url_open2           = ijkhttphook_open,
    .url_read            = ijkhttphook_read,
    .url_seek            = ijkhttphook_seek,
    .url_close           = ijkhttphook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijkhttphook_context_class,
};
