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

    /* options */
    char           *http_hook;
    char *         app_ctx_intptr;
} Context;

static int ijksegment_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    AVAppIOControl io_control = {0};
    AVApplicationContext *app_ctx = (AVApplicationContext *)av_dict_strtoptr(c->app_ctx_intptr);
    int ret = -1;
    int segment_index = -1;

    av_strstart(arg, "ijksegment:", &arg);

    if (!arg || !*arg)
        return AVERROR_EXTERNAL;

    segment_index = (int)strtol(arg, NULL, 0);
    io_control.size = sizeof(io_control);
    io_control.segment_index = segment_index;
    strlcpy(io_control.url,    arg,    sizeof(io_control.url));

    if (app_ctx && io_control.segment_index < 0) {
        ret = AVERROR_EXTERNAL;
        goto fail;
    }
    ret = av_application_on_io_control(app_ctx, AVAPP_CTRL_WILL_CONCAT_SEGMENT_OPEN, &io_control);
    if (ret || !io_control.url[0]) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    av_dict_set_intptr(options, "ijkapplication", (uintptr_t )app_ctx, 0);
    av_dict_set_int(options, "ijkinject-segment-index", segment_index, 0);

    ret = ffurl_open_whitelist(&c->inner,
                               io_control.url,
                               flags,
                               &h->interrupt_callback,
                               options,
                               h->protocol_whitelist,
                               h->protocol_blacklist,
                               h);
    if (ret)
        goto fail;

    return 0;
fail:
    return ret;
}

static int ijksegment_close(URLContext *h)
{
    Context *c = h->priv_data;

    return ffurl_close(c->inner);
}

static int ijksegment_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;

    return ffurl_read(c->inner, buf, size);
}

static int64_t ijksegment_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;

    return ffurl_seek(c->inner, pos, whence);
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijkapplication", "AVApplicationContext", OFFSET(app_ctx_intptr), AV_OPT_TYPE_INT64, { .i64 = 0 }, INT64_MIN, INT64_MAX, .flags = D },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijksegment_context_class = {
    .class_name = "Inject",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkimp_ff_ijksegment_protocol = {
    .name                = "ijksegment",
    .url_open2           = ijksegment_open,
    .url_read            = ijksegment_read,
    .url_seek            = ijksegment_seek,
    .url_close           = ijksegment_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijksegment_context_class,
};
