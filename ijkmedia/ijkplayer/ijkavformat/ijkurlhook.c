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

    int64_t         logical_pos;
    int64_t         logical_size;

    IJKAVInject_OnUrlOpenData inject_data;
    const char     *scheme;
    const char     *inner_scheme;
    int             open_callback_id;

    /* options */
    int             inner_flags;
    AVDictionary   *inner_options;
    int64_t         opaque;
    int             segment_index;
    int64_t         test_fail_point;
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

static int ijkurlhook_call_inject(URLContext *h)
{
    Context *c = h->priv_data;
    IjkAVInjectCallback inject_callback = ijkav_get_inject_callback();
    void *opaque = ijkinject_get_opaque(h);
    int ret = 0;

    if (opaque && inject_callback) {
        av_log(h, AV_LOG_INFO, "url-hook %s\n", c->inject_data.url);
        c->inject_data.is_handled = 0;
        ret = inject_callback(opaque, c->open_callback_id, &c->inject_data, sizeof(c->inject_data));
        if (ret || !c->inject_data.url[0]) {
            ret = AVERROR_EXIT;
            goto fail;
        }
    }

fail:
    return ret;
}

static int ijkurlhook_reconnect(URLContext *h, AVDictionary *extra)
{
    Context *c = h->priv_data;
    int ret = 0;
    URLContext *new_url = NULL;
    AVDictionary *inner_options = NULL;

    assert(c->inner_options);
    av_dict_copy(&inner_options, c->inner_options, 0);
    if (extra)
        av_dict_copy(&inner_options, extra, 0);

    ret = ffurl_open(&new_url, c->inject_data.url, c->inner_flags, &h->interrupt_callback, &inner_options);
    if (ret)
        goto fail;

    ffurl_closep(&c->inner);

    c->inner        = new_url;
    h->is_streamed  = c->inner->is_streamed;
    c->logical_pos  = ffurl_seek(c->inner, 0, SEEK_CUR);
    if (c->inner->is_streamed)
        c->logical_size = -1;
    else
        c->logical_size = ffurl_seek(c->inner, 0, AVSEEK_SIZE);
    
fail:
    av_dict_free(&inner_options);
    return ret;
}

static int ijkurlhook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    av_strstart(arg, c->scheme, &arg);

    c->inner_flags = flags;

    if (options)
        av_dict_copy(&c->inner_options, *options, 0);
    av_dict_set_int(&c->inner_options, "ijkinject-opaque",        c->opaque, 0);
    av_dict_set_int(&c->inner_options, "ijkinject-segment-index", c->segment_index, 0);

    c->inject_data.size = sizeof(c->inject_data);
    c->inject_data.segment_index = c->segment_index;

    if (av_strstart(arg, c->inner_scheme, NULL)) {
        snprintf(c->inject_data.url, sizeof(c->inject_data.url), "%s", arg);
    } else {
        snprintf(c->inject_data.url, sizeof(c->inject_data.url), "%s%s", c->inner_scheme, arg);
    }

    ret = ijkurlhook_call_inject(h);
    if (ret)
        goto fail;

    ret = ijkurlhook_reconnect(h, NULL);
    if (ret)
        goto fail;

fail:
    return ret;
}

static int ijktcphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    c->scheme = "ijktcphook:";
    c->inner_scheme = "tcp:";
    c->open_callback_id = IJKAVINJECT_ON_TCP_OPEN;
    return ijkurlhook_open(h, arg, flags, options);
}

static int ijkhttphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    c->scheme = "ijkhttphook:";
    c->inner_scheme = "http:";
    c->open_callback_id = IJKAVINJECT_ON_HTTP_OPEN;
    return ijkurlhook_open(h, arg, flags, options);
}

static int ijkurlhook_close(URLContext *h)
{
    Context *c = h->priv_data;

    av_dict_free(&c->inner_options);
    return ffurl_closep(&c->inner);
}

static int ijkurlhook_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    int ret = ffurl_read(c->inner, buf, size);

    if (ret > 0) {
        c->logical_pos += ret;
        if (c->test_fail_point > 0 && c->logical_pos >= c->test_fail_point)
            return AVERROR(EIO);
    }

    return ret;
}

static int ijkurlhook_write(URLContext *h, const unsigned char *buf, int size)
{
    Context *c = h->priv_data;

    return ffurl_write(c->inner, buf, size);
}

static int64_t ijkurlhook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int64_t seek_ret = 0;

    seek_ret = ffurl_seek(c->inner, pos, whence);
    if (seek_ret < 0)
        return seek_ret;

    c->logical_pos = seek_ret;
    return seek_ret;
}

static int64_t ijkhttphook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int ret = 0;
    AVDictionary *extra_opts = NULL;

    if (whence == AVSEEK_SIZE)
        return c->logical_size;
    else if ((whence == SEEK_CUR && pos == 0) ||
             (whence == SEEK_SET && pos == c->logical_pos))
        return c->logical_pos;
    else if ((c->logical_size < 0 && whence == SEEK_END) || h->is_streamed)
        return AVERROR(ENOSYS);

    ret = ijkurlhook_call_inject(h);
    if (ret)
        goto fail;

    if (!c->inject_data.is_handled)
        return ijkurlhook_seek(c->inner, pos, whence);

    if (whence == SEEK_CUR)
        pos += c->logical_pos;
    else if (whence == SEEK_END)
        pos += c->logical_size;
    else if (whence != SEEK_SET)
        return AVERROR(EINVAL);
    if (pos < 0)
        return AVERROR(EINVAL);

    av_dict_set_int(&extra_opts, "offset", pos, 0);
    ret = ijkurlhook_reconnect(h, extra_opts);
    if (ret)
        goto fail;

    av_dict_free(&extra_opts);
    return c->logical_pos;
fail:
    av_dict_free(&extra_opts);
    return ret;
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption ijkurlhook_options[] = {
    { "ijkinject-opaque",           "private data of user, passed with custom callback",
        OFFSET(opaque),             IJKAV_OPTION_INT64(0, INT64_MIN, INT64_MAX) },
    { "ijkinject-segment-index",    "segment index of current url",
        OFFSET(segment_index),      IJKAV_OPTION_INT(0, 0, INT_MAX) },
    { "ijkurlhook-test-fail-point", "test fail point, in bytes",
        OFFSET(test_fail_point),    IJKAV_OPTION_INT(0, 0, INT_MAX) },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijktcphook_context_class = {
    .class_name = "TcpHook",
    .item_name  = av_default_item_name,
    .option     = ijkurlhook_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkff_ijktcphook_protocol = {
    .name                = "ijktcphook",
    .url_open2           = ijktcphook_open,
    .url_read            = ijkurlhook_read,
    .url_write           = ijkurlhook_write,
    .url_seek            = ijkurlhook_seek,
    .url_close           = ijkurlhook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijktcphook_context_class,
};

static const AVClass ijkhttphook_context_class = {
    .class_name = "HttpHook",
    .item_name  = av_default_item_name,
    .option     = ijkurlhook_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkff_ijkhttphook_protocol = {
    .name                = "ijkhttphook",
    .url_open2           = ijkhttphook_open,
    .url_read            = ijkurlhook_read,
    .url_write           = ijkurlhook_write,
    .url_seek            = ijkhttphook_seek,
    .url_close           = ijkurlhook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijkhttphook_context_class,
};
