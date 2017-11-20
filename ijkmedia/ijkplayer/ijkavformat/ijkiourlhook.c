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
#include "ijkiourl.h"
#include "ijkioprotocol.h"
#include "ijkplayer/ijkavutil/ijkutils.h"
#include "libavutil/log.h"

#include "libavutil/application.h"

typedef struct Context {
    IjkURLContext   *inner;

    int64_t         logical_pos;
    int64_t         logical_size;
    int             io_error;

    AVAppIOControl  app_io_ctrl;
    const char     *scheme;
    const char     *inner_scheme;
    IjkAVIOInterruptCB *ijkio_interrupt_callback;

    /* options */
    int             inner_flags;
    IjkAVDictionary *inner_options;
    int             segment_index;
    int64_t         test_fail_point;
    int64_t         test_fail_point_next;
    int64_t         app_ctx_intptr;
    int             abort_request;
    AVApplicationContext *app_ctx;
    IjkIOApplicationContext *ijkio_app_ctx;
} Context;

static int ijkio_cache_check_interrupt(IjkURLContext *h)
{
    Context *c = h->priv_data;
    if (!c)
        return 1;

    if (c->abort_request)
        return 1;

    if (c->ijkio_interrupt_callback && c->ijkio_interrupt_callback->callback &&
                    c->ijkio_interrupt_callback->callback(c->ijkio_interrupt_callback->opaque)) {
        c->abort_request = 1;
    }

    if (c->abort_request)
        return 1;

    return c->abort_request;
}

static int ijkio_urlhook_call_inject(IjkURLContext *h)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (ijkio_cache_check_interrupt(h)) {
        ret = IJKAVERROR_EXIT;
        goto fail;
    }

    if (c->app_ctx) {
        AVAppIOControl control_data_backup = c->app_io_ctrl;

        c->app_io_ctrl.is_handled = 0;
        c->app_io_ctrl.is_url_changed = 0;
        ret = av_application_on_io_control(c->app_ctx, AVAPP_CTRL_WILL_HTTP_OPEN, &c->app_io_ctrl);
        if (ret || !c->app_io_ctrl.url[0]) {
            ret = IJKAVERROR_EXIT;
            goto fail;
        }

        AVAppIOControl user_control_data = c->app_io_ctrl;
        if (strncmp(c->app_io_ctrl.url, "ffio:", strlen("ffio:"))) {
            snprintf(c->app_io_ctrl.url, sizeof(c->app_io_ctrl.url), "%s%s", "ffio:", user_control_data.url);
        }

        if (!c->app_io_ctrl.is_url_changed && strcmp(control_data_backup.url, c->app_io_ctrl.url)) {
            // force a url compare
            c->app_io_ctrl.is_url_changed = 1;
        }

        av_log(NULL, AV_LOG_INFO, "%s %s (%s)\n", h->prot->name, c->app_io_ctrl.url, c->app_io_ctrl.is_url_changed ? "changed" : "remain");
    }

    if (ijkio_cache_check_interrupt(h)) {
        ret = IJKAVERROR_EXIT;
        av_log(NULL, AV_LOG_ERROR, "%s %s (%s)\n", h->prot->name, c->app_io_ctrl.url, c->app_io_ctrl.is_url_changed ? "changed" : "remain");
        goto fail;
    }

fail:
    return ret;
}

static int ijkio_urlhook_reconnect(IjkURLContext *h, IjkAVDictionary *extra)
{
    Context *c = h->priv_data;
    int ret = 0;
    IjkURLContext *new_url = NULL;
    IjkAVDictionary *inner_options = NULL;

    c->test_fail_point_next += c->test_fail_point;

    assert(c->inner_options);
    ijk_av_dict_copy(&inner_options, c->inner_options, 0);
    if (extra)
        ijk_av_dict_copy(&inner_options, extra, 0);

    ret = ijkio_alloc_url(&new_url, c->app_io_ctrl.url);
    new_url->ijkio_app_ctx = c->ijkio_app_ctx;
    if (ret)
        goto fail0;

    ret = new_url->prot->url_open2(new_url, c->app_io_ctrl.url, c->inner_flags, &inner_options);
    if (ret)
        goto fail1;

    if (c->inner) {
        c->inner->prot->url_close(c->inner);
        ijk_av_freep(&c->inner->priv_data);
        ijk_av_freep(&c->inner);
    }

    c->inner        = new_url;
    c->logical_pos  = c->inner->prot->url_seek(c->inner, 0, SEEK_CUR);
    c->logical_size = c->inner->prot->url_seek(c->inner, 0, IJKAVSEEK_SIZE);
    c->io_error     = 0;
    if (inner_options) {
        ijk_av_dict_free(&inner_options);
    }
    return ret;

fail1:
    ijk_av_freep(&new_url->priv_data);
    ijk_av_freep(&new_url);
fail0:
    if (inner_options) {
        ijk_av_dict_free(&inner_options);
    }
    return ret;
}

static int ijkio_urlhook_init(IjkURLContext *h, const char *arg, int flags, IjkAVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;

    // ijk_av_strstart(arg, c->scheme, &arg);

    c->inner_flags = flags;

    if (options)
        ijk_av_dict_copy(&c->inner_options, *options, 0);

    ijk_av_dict_set_int(&c->inner_options, "ijkapplication", c->app_ctx_intptr, 0);
    ijk_av_dict_set_int(&c->inner_options, "ijkinject-segment-index", c->segment_index, 0);

    c->app_io_ctrl.size = sizeof(c->app_io_ctrl);
    c->app_io_ctrl.segment_index = c->segment_index;
    c->app_io_ctrl.retry_counter = 0;

    snprintf(c->app_io_ctrl.url, sizeof(c->app_io_ctrl.url), "%s", arg);

    return ret;
}

static int ijkio_httphook_close(IjkURLContext *h)
{
    Context *c = h->priv_data;
    int ret = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    ret = c->inner->prot->url_close(c->inner);
    if (c->inner_options) {
        ijk_av_dict_free(&c->inner_options);
    }
    ijk_av_freep(&c->inner->priv_data);
    ijk_av_freep(&c->inner);

    return ret;
}

static int ijkio_urlhook_read(IjkURLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (c->io_error < 0)
        return c->io_error;

    if (c->test_fail_point_next > 0 && c->logical_pos >= c->test_fail_point_next) {
        av_log(NULL, AV_LOG_ERROR, "test fail point:%"PRId64"\n", c->test_fail_point_next);
        c->io_error = IJKAVERROR(EIO);
        return IJKAVERROR(EIO);
    }

    ret = c->inner->prot->url_read(c->inner, buf, size);
    if (ret > 0)
        c->logical_pos += ret;
    else
        c->io_error = ret;

    return ret;
}

static int64_t ijkio_urlhook_seek(IjkURLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int64_t seek_ret = 0;

    seek_ret = c->inner->prot->url_seek(c->inner, pos, whence);
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

static int ijkio_httphook_reconnect_at(IjkURLContext *h, int64_t offset)
{
    int ret = 0;
    IjkAVDictionary *extra_opts = NULL;

    ijk_av_dict_set_int(&extra_opts, "offset", offset, 0);
    ijk_av_dict_set_int(&extra_opts, "dns_cache_clear", 1, 0);
    ret = ijkio_urlhook_reconnect(h, extra_opts);
    ijk_av_dict_free(&extra_opts);
    return ret;
}

static int ijkio_httphook_open(IjkURLContext *h, const char *arg, int flags, IjkAVDictionary **options)
{
    Context *c = h->priv_data;
    int ret = 0;
    IjkAVDictionaryEntry *t = NULL;
    c->ijkio_app_ctx = h->ijkio_app_ctx;
    c->ijkio_interrupt_callback = h->ijkio_app_ctx->ijkio_interrupt_callback;

    t = ijk_av_dict_get(*options, "ijkapplication", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->app_ctx_intptr = (int64_t)strtoll(t->value, NULL, 10);
        c->app_ctx = (AVApplicationContext *)(intptr_t)c->app_ctx_intptr;
    } else {
        goto fail;
    }

    t = ijk_av_dict_get(*options, "ijkinject-segment-index", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->segment_index = (int)strtoll(t->value, NULL, 10);
    }

    t = ijk_av_dict_get(*options, "ijkhttphook-test-fail-point", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->test_fail_point = (int64_t)strtoll(t->value, NULL, 10);
    }
    ijk_av_strstart(arg, "httphook:", &arg);
    ret = ijkio_urlhook_init(h, arg, flags, options);
    if (ret)
        goto fail;

    ret = ijkio_urlhook_call_inject(h);
    if (ret)
        goto fail;

    ret = ijkio_urlhook_reconnect(h, NULL);

    while (ret && c->abort_request == 0) {
        int inject_ret = 0;

        switch (ret) {
            case IJKAVERROR_EXIT:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        inject_ret = ijkio_urlhook_call_inject(h);
        if (inject_ret) {
            ret = IJKAVERROR_EXIT;
            goto fail;
        }

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(NULL, AV_LOG_INFO, "%s: will reconnect at start\n", __func__);
        ret = ijkio_httphook_reconnect_at(h, 0);
        av_log(NULL, AV_LOG_INFO, "%s: did reconnect at start: %d\n", __func__, ret);
    }

fail:
    return ret;
}

static int ijkio_httphook_read(IjkURLContext *h, unsigned char *buf, int size)
{
    Context *c = h->priv_data;
    int ret = 0;
    int active_reconnect = c->ijkio_app_ctx->active_reconnect;

    c->app_io_ctrl.retry_counter = 0;

    if (active_reconnect == 0) {
        ret = ijkio_urlhook_read(h, buf, size);
    }

    while ((active_reconnect || ret < 0) && c->logical_pos < c->logical_size && c->abort_request == 0) {
        switch (ret) {
            case IJKAVERROR_EXIT:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        ret = ijkio_urlhook_call_inject(h);
        c->ijkio_app_ctx->active_reconnect = active_reconnect = 0;
        if (ret)
            goto fail;

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(NULL, AV_LOG_INFO, "%s: will reconnect(%d) at %"PRId64"\n", __func__, c->app_io_ctrl.retry_counter, c->logical_pos);
        ret = ijkio_httphook_reconnect_at(h, c->logical_pos);
        av_log(NULL, AV_LOG_INFO, "%s: did reconnect(%d) at %"PRId64": %d\n", __func__, c->app_io_ctrl.retry_counter, c->logical_pos, ret);
        if (ret < 0)
            continue;

        ret = ijkio_urlhook_read(h, buf, size);
    }

fail:
    if (ret <= 0) {
        c->io_error = ret;
    }
    return ret;
}

static int64_t ijkio_httphook_reseek_at(IjkURLContext *h, int64_t pos, int whence, int force_reconnect)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (!force_reconnect)
        return ijkio_urlhook_seek(h, pos, whence);

    if (whence == SEEK_CUR)
        pos += c->logical_pos;
    else if (whence == SEEK_END)
        pos += c->logical_size;
    else if (whence != SEEK_SET)
        return IJKAVERROR(EINVAL);
    if (pos < 0)
        return IJKAVERROR(EINVAL);

    ret = ijkio_httphook_reconnect_at(h, pos);
    if (ret) {
        c->io_error = ret;
        return ret;
    }

    c->io_error = 0;
    return c->logical_pos;
}

static int64_t ijkio_httphook_seek(IjkURLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int     ret      = 0;
    int64_t seek_ret = -1;

    if (whence == IJKAVSEEK_SIZE)
        return c->logical_size;
    else if ((whence == SEEK_CUR && pos == 0) ||
             (whence == SEEK_SET && pos == c->logical_pos))
        return c->logical_pos;
    else if ((c->logical_size < 0 && whence == SEEK_END))
        return IJKAVERROR(ENOSYS);

    c->app_io_ctrl.retry_counter = 0;
    ret = ijkio_urlhook_call_inject(h);
    if (ret) {
        ret = IJKAVERROR_EXIT;
        goto fail;
    }

    seek_ret = ijkio_httphook_reseek_at(h, pos, whence, c->app_io_ctrl.is_url_changed);
    while (seek_ret < 0 && c->abort_request == 0) {
        switch (seek_ret) {
            case IJKAVERROR_EXIT:
            case IJKAVERROR_EOF:
                goto fail;
        }

        c->app_io_ctrl.retry_counter++;
        ret = ijkio_urlhook_call_inject(h);
        if (ret) {
            ret = IJKAVERROR_EXIT;
            goto fail;
        }

        if (!c->app_io_ctrl.is_handled)
            goto fail;

        av_log(NULL, AV_LOG_INFO, "%s: will reseek(%d) at pos=%"PRId64", whence=%d\n", __func__, c->app_io_ctrl.retry_counter, pos, whence);
        seek_ret = ijkio_httphook_reseek_at(h, pos, whence, c->app_io_ctrl.is_url_changed);
        av_log(NULL, AV_LOG_INFO, "%s: did reseek(%d) at pos=%"PRId64", whence=%d: %"PRId64"\n", __func__, c->app_io_ctrl.retry_counter, pos, whence, seek_ret);
    }

    if (c->test_fail_point)
        c->test_fail_point_next = c->logical_pos + c->test_fail_point;
    c->io_error = 0;
    if (seek_ret < 0) {
        return seek_ret;
    }
    return c->logical_pos;
fail:
    return ret;
}

static int ijkio_httphook_pause(IjkURLContext *h) {
    Context *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);
    c->abort_request = 1;

    if (c->inner->prot->url_pause) {
        ret = c->inner->prot->url_pause(c->inner);
    }

    return ret;
}

static int ijkio_httphook_resume(IjkURLContext *h) {
    Context *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (c->inner->prot->url_resume) {
        ret = c->inner->prot->url_resume(c->inner);
        if (ret != 0) {
            return ret;
        }
    }
    c->abort_request = 0;
    return ret;
}

IjkURLProtocol ijkio_httphook_protocol = {
    .name                = "ijkiohttphook",
    .url_open2           = ijkio_httphook_open,
    .url_read            = ijkio_httphook_read,
    .url_seek            = ijkio_httphook_seek,
    .url_close           = ijkio_httphook_close,
    .url_pause           = ijkio_httphook_pause,
    .url_resume          = ijkio_httphook_resume,
    .priv_data_size      = sizeof(Context),
};
