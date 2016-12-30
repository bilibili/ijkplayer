/*
 * Copyright (c) 2016 Raymond Zheng <raymondzheng1412@gmail.com>
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

#include "ijkiomanager.h"
#include "ijkplayer/ijkavutil/ijkdict.h"

typedef struct Context {
    AVClass *class;
    int64_t *io_manager_ctx;
} Context;

static int ijkio_copy_options(IjkAVDictionary **dst, AVDictionary *src) {
    AVDictionaryEntry *t = NULL;

    while ((t = av_dict_get(src, "", t, AV_DICT_IGNORE_SUFFIX))) {
        int ret = ijk_av_dict_set(dst, t->key, t->value, 0);
        if (ret < 0)
            return ret;
    }

    return 0;
}

static int ijkio_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = -1;

    if (!c || !c->io_manager_ctx)
        return -1;

    IjkIOManagerContext *manager_ctx = (IjkIOManagerContext *)(c->io_manager_ctx);
    manager_ctx->ijkio_interrupt_callback = (IjkAVIOInterruptCB *)&(h->interrupt_callback);

    av_strstart(arg, "ijkio:", &arg);
    IjkAVDictionary *opts = NULL;
    ijkio_copy_options(&opts, *options);

    manager_ctx->cur_ffmpeg_ctx = c;

    ret = ijkio_manager_io_open(manager_ctx, arg, flags, &opts);
    ijk_av_dict_free(&opts);

    if (ret < 0) {
        ijkio_manager_io_close(manager_ctx);
    }

    return ret;
}

static int ijkio_read(URLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;

    if (!c || !c->io_manager_ctx)
        return -1;

    ((IjkIOManagerContext *)(c->io_manager_ctx))->cur_ffmpeg_ctx  = c;
    return ijkio_manager_io_read((IjkIOManagerContext *)(c->io_manager_ctx), buf, size);
}

static int64_t ijkio_seek(URLContext *h, int64_t offset, int whence)
{
    Context *c = h->priv_data;

    if (!c || !c->io_manager_ctx)
        return -1;

    ((IjkIOManagerContext *)(c->io_manager_ctx))->cur_ffmpeg_ctx  = c;
    return ijkio_manager_io_seek((IjkIOManagerContext *)(c->io_manager_ctx), offset, whence);
}

static int ijkio_close(URLContext *h)
{
    Context *c = h->priv_data;

    if (!c || !c->io_manager_ctx)
        return -1;

    ((IjkIOManagerContext *)(c->io_manager_ctx))->cur_ffmpeg_ctx  = c;
    return ijkio_manager_io_close((IjkIOManagerContext *)(c->io_manager_ctx));
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijkiomanager", "IjkIOManagerContext", OFFSET(io_manager_ctx), AV_OPT_TYPE_INT64, { .i64 = 0 }, INT64_MIN, INT64_MAX, .flags = D },
};

#undef D
#undef OFFSET

static const AVClass ijkio_context_class = {
    .class_name = "IjkIo",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkimp_ff_ijkio_protocol = {
    .name                = "ijkio",
    .url_open2           = ijkio_open,
    .url_read            = ijkio_read,
    .url_seek            = ijkio_seek,
    .url_close           = ijkio_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijkio_context_class,
};
