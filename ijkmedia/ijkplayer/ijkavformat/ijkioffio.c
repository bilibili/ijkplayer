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
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "ijkplayer/ijkavutil/ijkutils.h"
#include "ijkiourl.h"

#include <stdint.h>

typedef struct IjkIOFFioContext {
    URLContext *inner;
} IjkIOFFioContext;

static int ijkio_copy_options(AVDictionary **dst, IjkAVDictionary *src) {
    IjkAVDictionaryEntry *t = NULL;

    while ((t = ijk_av_dict_get(src, "", t, IJK_AV_DICT_IGNORE_SUFFIX))) {
        int ret = av_dict_set(dst, t->key, t->value, 0);
        if (ret < 0)
            return ret;
    }

    return 0;
}

static int ijkio_ffio_open(IjkURLContext *h, const char *url, int flags, IjkAVDictionary **options) {
    int ret = -1;

    IjkIOFFioContext *c= h->priv_data;
    if (!c)
        return -1;

    AVDictionary *opts = NULL;
    ijkio_copy_options(&opts, *options);

    av_strstart(url, "ffio:", &url);
    if (h->ijkio_app_ctx) {
        ret = ffurl_open_whitelist(&c->inner, url, flags, (AVIOInterruptCB *)h->ijkio_app_ctx->ijkio_interrupt_callback,
                                &opts, NULL, NULL, NULL);
    } else {
        ret = -1;
    }

    av_dict_free(&opts);

    return ret;
}

static int ijkio_ffio_read(IjkURLContext *h, unsigned char *buf, int size) {
    if (!h)
        return -1;

    IjkIOFFioContext *c= h->priv_data;
    if (!c || !c->inner)
        return -1;

    return ffurl_read(c->inner, buf, size);
}

static int64_t ijkio_ffio_seek(IjkURLContext *h, int64_t offset, int whence) {
    if (!h)
        return -1;

    IjkIOFFioContext *c= h->priv_data;

    if (!c || !c->inner)
        return -1;

    return ffurl_seek(c->inner, offset, whence);
}

static int ijkio_ffio_close(IjkURLContext *h) {
    if (!h)
        return -1;

    IjkIOFFioContext *c= h->priv_data;

    if (!c || !c->inner)
        return -1;

    return ffurl_close(c->inner);
}

IjkURLProtocol ijkio_ffio_protocol = {
    .name                = "ijkffio",
    .url_open2           = ijkio_ffio_open,
    .url_read            = ijkio_ffio_read,
    .url_seek            = ijkio_ffio_seek,
    .url_close           = ijkio_ffio_close,
    .priv_data_size      = sizeof(IjkIOFFioContext),
};
