/*
 * Copyright (c) 2015 Bilibili
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

#include "libavutil/application.h"

typedef struct Context {
    AVClass        *class;
    URLContext     *inner;

    int64_t         logical_pos;
    int64_t         logical_size;
    int             io_error;

    AVAppIOControl  app_io_ctrl;
    const char     *scheme;
    const char     *inner_scheme;

    /* options */
    int             inner_flags;
    AVDictionary   *inner_options;
    int             segment_index;
    int64_t         test_fail_point;
    int64_t         test_fail_point_next;
    char*         app_ctx_intptr;
    AVApplicationContext *app_ctx;
} Context;

static int ijkurlhook_call_inject(URLContext *h)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (ff_check_interrupt(&h->interrupt_callback)) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    if (c->app_ctx) {
        AVAppIOControl control_data_backup = c->app_io_ctrl;

        c->app_io_ctrl.is_handled = 0;
        c->app_io_ctrl.is_url_changed = 0;
        ret = av_application_on_io_control(c->app_ctx, AVAPP_CTRL_WILL_HTTP_OPEN, &c->app_io_ctrl);
        if (ret || !c->app_io_ctrl.url[0]) {
            ret = AVERROR_EXIT;
            goto fail;
        }
        if (!c->app_io_ctrl.is_url_changed && strcmp(control_data_backup.url, c->app_io_ctrl.url)) {
            // force a url compare
            c->app_io_ctrl.is_url_changed = 1;
        }

        av_log(h, AV_LOG_INFO, "%s %s (%s)\n", h->prot->name, c->app_io_ctrl.url, c->app_io_ctrl.is_url_changed ? "changed" : "remain");
    }

    if (ff_check_interrupt(&h->interrupt_callback)) {
        ret = AVERROR_EXIT;
        av_log(h, AV_LOG_ERROR, "%s %s (%s)\n", h->prot->name, c->app_io_ctrl.url, c->app_io_ctrl.is_url_changed ? "changed" : "remain");
        goto fail;
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

    c->test_fail_point_next += c->test_fail_point;

    assert(c->inner_options);
    av_dict_copy(&inner_options, c->inner_options, 0);
    if (extra)
        av_dict_copy(&inner_options, extra, 0);

    ret = ffurl_open_whitelist(&new_url,
                               c->app_io_ctrl.url,
                               c->inner_flags,
                               &h->interrupt_callback,
                               &inner_options,
                               h->protocol_whitelist,
                               h->protocol_blacklist,
                               h);
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

    c->io_error = 0;
fail:
    av_dict_free(&inner_options);
    return ret;
}

static int ijkurlhook_init(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    av_strstart(arg, c->scheme, &arg);

    c->inner_flags = flags;

    if (options)
        av_dict_copy(&c->inner_options, *options, 0);

    av_dict_set_intptr(&c->inner_options, "ijkapplication", (uintptr_t )c->app_ctx, 0);
    av_dict_set_int(&c->inner_options, "ijkinject-segment-index", c->segment_index, 0);

    c->app_io_ctrl.size = sizeof(c->app_io_ctrl);
    c->app_io_ctrl.segment_index = c->segment_index;
    c->app_io_ctrl.retry_counter = 0;

    if (av_strstart(arg, c->inner_scheme, NULL)) {
        snprintf(c->app_io_ctrl.url, sizeof(c->app_io_ctrl.url), "%s", arg);
    } else {
        snprintf(c->app_io_ctrl.url, sizeof(c->app_io_ctrl.url), "%s%s", c->inner_scheme, arg);
    }

    return ret;
}

static int ijktcphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    c->app_ctx = (AVApplicationContext *)av_dict_strtoptr(c->app_ctx_intptr);
    c->scheme = "ijktcphook:";
    c->inner_scheme = "tcp:";
    ret = ijkurlhook_init(h, arg, flags, options);
    if (ret)
        goto fail;

    ret = ijkurlhook_reconnect(h, NULL);
    if (ret)
        goto fail;

fail:
    return ret;
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
    int ret = 0;

    if (c->io_error < 0)
        return c->io_error;

    if (c->test_fail_point_next > 0 && c->logical_pos >= c->test_fail_point_next) {
        av_log(h, AV_LOG_ERROR, "test fail point:%"PRId64"\n", c->test_fail_point_next);
        c->io_error = AVERROR(EIO);
        return AVERROR(EIO);
    }

    ret = ffurl_read(c->inner, buf, size);
    if (ret > 0)
        c->logical_pos += ret;
    else
        c->io_error = ret;

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
    if (seek_ret < 0) {
        c->io_error = (int)seek_ret;
        return seek_ret;
    }

    c->logical_pos = seek_ret;
    if (c->test_fail_point)
        c->test_fail_point_next = c->logical_pos + c->test_fail_point;

    c->io_error = 0;
    return seek_ret;
}

static int ijkhttphook_reconnect_at(URLContext *h, int64_t offset)
{
    int           ret        = 0;
    AVDictionary *extra_opts = NULL;

    av_dict_set_int(&extra_opts, "offset", offset, 0);
    av_dict_set_int(&extra_opts, "dns_cache_clear", 1, 0);
    ret = ijkurlhook_reconnect(h, extra_opts);
    av_dict_free(&extra_opts);
    return ret;
}

static int ijkhttphook_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    c->app_ctx = (AVApplicationContext *)av_dict_strtoptr(c->app_ctx_intptr);
    c->scheme = "ijkhttphook:";
    if (av_stristart(arg, "ijkhttphook:https:", NULL))
        c->inner_scheme = "https:";
    else
        c->inner_scheme = "http:";

    ret = ijkurlhook_init(h, arg, flags, options);
    if (ret)
        goto fail;

    ret = ijkurlhook_call_inject(h);
    if (ret)
        goto fail;

    ret = ijkurlhook_reconnect(h, NULL);
    while (ret) {
        int inject_ret = 0;

        switch (ret) {
            case AVERROR_EXIT:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        inject_ret = ijkurlhook_call_inject(h);
        if (inject_ret) {
            ret = AVERROR_EXIT;
            goto fail;
        }

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(h, AV_LOG_INFO, "%s: will reconnect at start\n", __func__);
        ret = ijkhttphook_reconnect_at(h, 0);
        av_log(h, AV_LOG_INFO, "%s: did reconnect at start: %d\n", __func__, ret);
    }

fail:
    return ret;
}

static int ijkhttphook_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    int ret = 0;

    c->app_io_ctrl.retry_counter = 0;

    ret = ijkurlhook_read(h, buf, size);
    while (ret < 0 && !h->is_streamed && c->logical_pos < c->logical_size) {
        switch (ret) {
            case AVERROR_EXIT:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        ret = ijkurlhook_call_inject(h);
        if (ret)
            goto fail;

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(h, AV_LOG_INFO, "%s: will reconnect(%d) at %"PRId64"\n", __func__, c->app_io_ctrl.retry_counter, c->logical_pos);
        ret = ijkhttphook_reconnect_at(h, c->logical_pos);
        av_log(h, AV_LOG_INFO, "%s: did reconnect(%d) at %"PRId64": %d\n", __func__, c->app_io_ctrl.retry_counter, c->logical_pos, ret);
        if (ret < 0)
            continue;

        ret = ijkurlhook_read(h, buf, size);
    }

fail:
    if (ret <= 0) {
        c->io_error = ret;
    }
    return ret;
}

static int64_t ijkhttphook_reseek_at(URLContext *h, int64_t pos, int whence, int force_reconnect)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (!force_reconnect)
        return ijkurlhook_seek(h, pos, whence);

    if (whence == SEEK_CUR)
        pos += c->logical_pos;
    else if (whence == SEEK_END)
        pos += c->logical_size;
    else if (whence != SEEK_SET)
        return AVERROR(EINVAL);
    if (pos < 0)
        return AVERROR(EINVAL);

    ret = ijkhttphook_reconnect_at(h, pos);
    if (ret) {
        c->io_error = ret;
        return ret;
    }

    c->io_error = 0;
    return c->logical_pos;
}

static int64_t ijkhttphook_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int     ret      = 0;
    int64_t seek_ret = -1;

    if (whence == AVSEEK_SIZE)
        return c->logical_size;
    else if ((whence == SEEK_CUR && pos == 0) ||
             (whence == SEEK_SET && pos == c->logical_pos))
        return c->logical_pos;
    else if ((c->logical_size < 0 && whence == SEEK_END) || h->is_streamed)
        return AVERROR(ENOSYS);

    c->app_io_ctrl.retry_counter = 0;
    ret = ijkurlhook_call_inject(h);
    if (ret) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    seek_ret = ijkhttphook_reseek_at(h, pos, whence, c->app_io_ctrl.is_url_changed);
    while (seek_ret < 0) {
        switch (seek_ret) {
            case AVERROR_EXIT:
            case AVERROR_EOF:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        ret = ijkurlhook_call_inject(h);
        if (ret) {
            ret = AVERROR_EXIT;
            goto fail;
        }

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(h, AV_LOG_INFO, "%s: will reseek(%d) at pos=%"PRId64", whence=%d\n", __func__, c->app_io_ctrl.retry_counter, pos, whence);
        seek_ret = ijkhttphook_reseek_at(h, pos, whence, c->app_io_ctrl.is_url_changed);
        av_log(h, AV_LOG_INFO, "%s: did reseek(%d) at pos=%"PRId64", whence=%d: %"PRId64"\n", __func__, c->app_io_ctrl.retry_counter, pos, whence, seek_ret);
    }

    if (c->test_fail_point)
        c->test_fail_point_next = c->logical_pos + c->test_fail_point;
    c->io_error = 0;
    return c->logical_pos;
fail:
    return ret;
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption ijktcphook_options[] = {
    { "ijktcphook-test-fail-point",     "test fail point, in bytes",
        OFFSET(test_fail_point),        AV_OPT_TYPE_INT,   {.i64 = 0}, 0,         INT_MAX, D },
    { "ijkapplication", "AVApplicationContext", OFFSET(app_ctx_intptr), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, .flags = D },

    { NULL }
};

static const AVOption ijkhttphook_options[] = {
    { "ijkinject-segment-index",        "segment index of current url",
        OFFSET(segment_index),          AV_OPT_TYPE_INT,   {.i64 = 0}, 0,         INT_MAX, D },
    { "ijkhttphook-test-fail-point",    "test fail point, in bytes",
        OFFSET(test_fail_point),        AV_OPT_TYPE_INT,   {.i64 = 0}, 0,         INT_MAX, D },
    { "ijkapplication", "AVApplicationContext", OFFSET(app_ctx_intptr), AV_OPT_TYPE_STRING, { .str = NULL }, 0, 0, .flags = D },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijktcphook_context_class = {
    .class_name = "TcpHook",
    .item_name  = av_default_item_name,
    .option     = ijktcphook_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkimp_ff_ijktcphook_protocol = {
    .name                = "ijktcphook",
    .url_open2           = ijktcphook_open,
    .url_read            = ijkurlhook_read,
    .url_write           = ijkurlhook_write,
    .url_close           = ijkurlhook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijktcphook_context_class,
};

static const AVClass ijkhttphook_context_class = {
    .class_name = "HttpHook",
    .item_name  = av_default_item_name,
    .option     = ijkhttphook_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkimp_ff_ijkhttphook_protocol = {
    .name                = "ijkhttphook",
    .url_open2           = ijkhttphook_open,
    .url_read            = ijkhttphook_read,
    .url_write           = ijkurlhook_write,
    .url_seek            = ijkhttphook_seek,
    .url_close           = ijkurlhook_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijkhttphook_context_class,
};
