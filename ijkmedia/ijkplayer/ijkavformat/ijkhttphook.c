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
    int             avio_flag;

    int64_t         logic_pos;
    int64_t         logic_size;
    int             seekable;

    IJKAVInject_OnUrlOpenData inject_data;

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

static int open_inner(URLContext *h)
{
    Context                *c               = h->priv_data;
    void                   *opaque          = ijkinject_get_opaque(h);
    IjkAVInjectCallback     inject_callback = ijkav_get_inject_callback();
    AVDictionary           *tmp_opts        = NULL;
    int ret = -1;

    if (ff_check_interrupt(&h->interrupt_callback)) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    if (c->inject_data.retry_counter > 0) {
        av_log(h, AV_LOG_INFO, "http-hook-retry %s (%d)\n", c->inject_data.url, c->inject_data.retry_counter);
        ret = inject_callback(opaque, IJKAVINJECT_ON_HTTP_RETRY, &c->inject_data, sizeof(c->inject_data));
        if (ret || !c->inject_data.url[0]) {
            ret = AVERROR_EXIT;
            goto fail;
        }
    }

    ffurl_closep(&c->inner);

    if (c->open_opts)
        av_dict_copy(&tmp_opts, c->open_opts, 0);

    ret = ffurl_open(&c->inner, c->inject_data.url, c->open_flags, &h->interrupt_callback, &tmp_opts);
    if (ret)
        goto fail;

    c->logic_size = ffurl_size(c->inner);
    c->seekable   = c->logic_size > 0 ? 1 : 0;

    av_dict_free(&tmp_opts);
    return 0;
fail:
    av_dict_free(&tmp_opts);
    ffurl_closep(&c->inner);
    return ret;
}

static int ijkhttphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    c->open_flags = flags;
    if (options) {
        av_dict_set_int(options, "ijkinject-opaque",        c->opaque, 0);
        av_dict_set_int(options, "ijkinject-segment-index", c->segment_index, 0);
        av_dict_copy(&c->open_opts, *options, 0);
    }

    av_strstart(arg, "ijkhttphook:", &arg);

    c->inject_data.size = sizeof(c->inject_data);
    c->inject_data.segment_index = c->segment_index;
    if (av_strstart(arg, "http:", NULL)) {
        snprintf(c->inject_data.url, sizeof(c->inject_data.url), "%s", arg);
    } else {
        snprintf(c->inject_data.url, sizeof(c->inject_data.url), "http:%s", arg);
    }

    ret = open_inner(h);
    while (ret) {
        c->inject_data.retry_counter++;

        // no EOF in live mode
        switch (ret) {
            case AVERROR_EXIT:
            case AVERROR_EOF:
                goto fail;
        }

        ret = open_inner(h);
    }

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
    int     ret = AVERROR_EXIT;
    int64_t seek_ret = -1;

    if (c->inner) {
        ret = ffurl_read(c->inner, buf, size);
        if (ret >= 0)
            goto success;
    }

    while (ret < 0) {
        // no EOF in live mode
        switch (ret) {
            case AVERROR_EXIT:
            case AVERROR_EOF:
                goto fail;
        }

        if (!c->seekable)
            goto fail;

        c->inject_data.retry_counter++;

        ret = open_inner(h);
        if (ret)
            continue;

        seek_ret = ffurl_seek(h, c->logic_pos, SEEK_SET);
        if (seek_ret < 0) {
            ret = (int)seek_ret;
            continue;
        } else if (seek_ret != c->logic_pos) {
            ret = AVERROR_INVALIDDATA;
            continue;
        }

        ret = ffurl_read(c->inner, buf, size);
        if (ret >= 0)
            break;
    }

success:
    if (ret > 0)
        c->logic_pos += ret;
fail:
    return ret;
}

static int64_t ijkhttphook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int64_t seek_ret = AVERROR_EXIT;

    if (c->inner) {
        seek_ret = ffurl_seek(c->inner, pos, whence);
        if (seek_ret >= 0)
            goto success;
    }

    while (seek_ret < 0) {
        // no EOF in live mode
        switch (seek_ret) {
            case AVERROR_EXIT:
            case AVERROR_EOF:
                goto fail;
        }

        if (!c->seekable)
            goto fail;

        c->inject_data.retry_counter++;

        seek_ret = open_inner(h);
        if (seek_ret)
            continue;

        seek_ret = ffurl_seek(h, c->logic_pos, whence);
        if (seek_ret < 0) {
            continue;
        } else if (seek_ret != c->logic_pos) {
            seek_ret = AVERROR_INVALIDDATA;
            continue;
        }
    }

success:
    if (seek_ret >= 0)
        c->logic_pos = ffurl_seek(c->inner, 0, SEEK_CUR);
fail:
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
