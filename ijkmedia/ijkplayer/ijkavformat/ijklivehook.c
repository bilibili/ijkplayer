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

#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/opt.h"

#include "ijkplayer/ijkavutil/opt.h"

#include "ijkavformat.h"
#include "libavutil/application.h"

typedef struct {
    AVClass         *class;
    AVFormatContext *inner;

    AVAppIOControl   io_control;
    int              discontinuity;
    int              error;

    /* options */
    AVDictionary   *open_opts;
    int64_t         app_ctx_intptr;
    AVApplicationContext *app_ctx;
} Context;

static int ijkurlhook_call_inject(AVFormatContext *h)
{
    Context *c = h->priv_data;
    int ret = 0;

    if (ff_check_interrupt(&h->interrupt_callback)) {
        ret = AVERROR_EXIT;
        goto fail;
    }

    if (c->app_ctx) {
        av_log(h, AV_LOG_INFO, "livehook %s\n", c->io_control.url);
        c->io_control.is_handled = 0;
        ret = av_application_on_io_control(c->app_ctx, AVAPP_CTRL_WILL_LIVE_OPEN, &c->io_control);
        if (ret || !c->io_control.url[0]) {
            ret = AVERROR_EXIT;
            goto fail;
        }
    }

    if (ff_check_interrupt(&h->interrupt_callback)) {
        ret = AVERROR_EXIT;
        goto fail;
    }

fail:
    return ret;
}

static int ijklivehook_probe(AVProbeData *probe)
{
    if (av_strstart(probe->filename, "ijklivehook:", NULL))
        return AVPROBE_SCORE_MAX;

    return 0;
}

static int ijklivehook_read_close(AVFormatContext *avf)
{
    Context *c = avf->priv_data;

    avformat_close_input(&c->inner);
    return 0;
}

// FIXME: install libavformat/internal.h
int ff_alloc_extradata(AVCodecParameters *par, int size);

static int copy_stream_props(AVStream *st, AVStream *source_st)
{
    int ret;

    if (st->codecpar->codec_id || !source_st->codecpar->codec_id) {
        if (st->codecpar->extradata_size < source_st->codecpar->extradata_size) {
            if (st->codecpar->extradata) {
                av_freep(&st->codecpar->extradata);
                st->codecpar->extradata_size = 0;
            }
            ret = ff_alloc_extradata(st->codecpar,
                                     source_st->codecpar->extradata_size);
            if (ret < 0)
                return ret;
        }
        memcpy(st->codecpar->extradata, source_st->codecpar->extradata,
               source_st->codecpar->extradata_size);
        return 0;
    }
    if ((ret = avcodec_parameters_copy(st->codecpar, source_st->codecpar)) < 0)
        return ret;
    st->r_frame_rate        = source_st->r_frame_rate;
    st->avg_frame_rate      = source_st->avg_frame_rate;
    st->time_base           = source_st->time_base;
    st->sample_aspect_ratio = source_st->sample_aspect_ratio;

    av_dict_copy(&st->metadata, source_st->metadata, 0);
    return 0;
}

static int open_inner(AVFormatContext *avf)
{
    Context         *c          = avf->priv_data;
    AVDictionary    *tmp_opts   = NULL;
    AVFormatContext *new_avf    = NULL;
    int ret = -1;
    int i   = 0;
    AVDictionaryEntry *t = NULL;
    int fps_flag = 0;

    new_avf = avformat_alloc_context();
    if (!new_avf) {
        ret = AVERROR(ENOMEM);
        goto fail;
    }

    if (c->open_opts)
        av_dict_copy(&tmp_opts, c->open_opts, 0);

    av_dict_set_int(&tmp_opts, "probesize",         avf->probesize, 0);
    av_dict_set_int(&tmp_opts, "formatprobesize",   avf->format_probesize, 0);
    av_dict_set_int(&tmp_opts, "analyzeduration",   avf->max_analyze_duration, 0);
    av_dict_set_int(&tmp_opts, "fpsprobesize",      avf->fps_probe_size, 0);
    av_dict_set_int(&tmp_opts, "max_ts_probe",      avf->max_ts_probe, 0);

    t = av_dict_get(tmp_opts, "skip-calc-frame-rate", NULL, AV_DICT_MATCH_CASE);
    if (t) {
        fps_flag = (int) strtol(t->value, NULL, 10);
        if (fps_flag > 0) {
            av_dict_set_int(&new_avf->metadata, "skip-calc-frame-rate", fps_flag, 0);
        }
    }

    new_avf->interrupt_callback = avf->interrupt_callback;
    ret = avformat_open_input(&new_avf, c->io_control.url, NULL, &tmp_opts);
    if (ret < 0)
        goto fail;

    ret = avformat_find_stream_info(new_avf, NULL);
    if (ret < 0)
        goto fail;

    for (i = 0; i < new_avf->nb_streams; i++) {
        AVStream *st = avformat_new_stream(avf, NULL);
        if (!st) {
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = copy_stream_props(st, new_avf->streams[i]);
        if (ret < 0)
            goto fail;
    }

    avformat_close_input(&c->inner);
    c->inner = new_avf;
    new_avf = NULL;
    ret = 0;
fail:
    av_dict_free(&tmp_opts);
    avformat_close_input(&new_avf);
    return ret;
}

static int ijklivehook_read_header(AVFormatContext *avf, AVDictionary **options)
{
    Context    *c           = avf->priv_data;
    const char *inner_url   = NULL;
    int         ret         = -1;

    c->app_ctx = (AVApplicationContext *)(intptr_t)c->app_ctx_intptr;
    av_strstart(avf->filename, "ijklivehook:", &inner_url);

    c->io_control.size = sizeof(c->io_control);
    strlcpy(c->io_control.url, inner_url, sizeof(c->io_control.url));

    if (av_stristart(c->io_control.url, "rtmp", NULL) ||
        av_stristart(c->io_control.url, "rtsp", NULL)) {
        // There is total different meaning for 'timeout' option in rtmp
        av_log(avf, AV_LOG_WARNING, "remove 'timeout' option for rtmp.\n");
        av_dict_set(options, "timeout", NULL, 0);
    }

    if (options)
        av_dict_copy(&c->open_opts, *options, 0);

    c->io_control.retry_counter = 0;
    ret = ijkurlhook_call_inject(avf);
    if (ret) {
        ret = AVERROR_EXIT;
        goto fail;
    }
    ret = open_inner(avf);
    while (ret < 0) {
        // no EOF in live mode
        switch (ret) {
            case AVERROR_EXIT:
                goto fail;
        }

        c->io_control.retry_counter++;
        ret = ijkurlhook_call_inject(avf);
        if (ret) {
            ret = AVERROR_EXIT;
            goto fail;
        }

        c->discontinuity = 1;
        ret = open_inner(avf);
    }

    return 0;
fail:
    return ret;
}

static int ijklivehook_read_packet(AVFormatContext *avf, AVPacket *pkt)
{
    Context *c   = avf->priv_data;
    int      ret = -1;

    if (c->error)
        return c->error;

    if (c->inner)
        ret = av_read_frame(c->inner, pkt);

    c->io_control.retry_counter = 0;
    while (ret < 0) {
        if (c->inner && c->inner->pb && c->inner->pb->error && avf->pb)
            avf->pb->error = c->inner->pb->error;

        // no EOF in live mode
        switch (ret) {
            case AVERROR_EXIT:
                c->error = AVERROR_EXIT;
                goto fail;
            case AVERROR(EAGAIN):
                goto continue_read;
        }

        c->io_control.retry_counter++;
        ret = ijkurlhook_call_inject(avf);
        if (ret) {
            ret = AVERROR_EXIT;
            goto fail;
        }

        c->discontinuity = 1;
        ret = open_inner(avf);
        if (ret)
            continue;

continue_read:
        ret = av_read_frame(c->inner, pkt);
    }

    if (c->discontinuity) {
        pkt->flags |= AV_PKT_FLAG_DISCONTINUITY;
        c->discontinuity = 0;
    }

    return 0;
fail:
    return ret;
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { "ijkapplication", "AVApplicationContext", OFFSET(app_ctx_intptr), AV_OPT_TYPE_INT64, { .i64 = 0 }, INT64_MIN, INT64_MAX, .flags = D },
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijklivehook_class = {
    .class_name = "LiveHook demuxer",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ijkff_ijklivehook_demuxer = {
    .name           = "ijklivehook",
    .long_name      = "Live Hook Controller",
    .flags          = AVFMT_NOFILE | AVFMT_TS_DISCONT,
    .priv_data_size = sizeof(Context),
    .read_probe     = ijklivehook_probe,
    .read_header2   = ijklivehook_read_header,
    .read_packet    = ijklivehook_read_packet,
    .read_close     = ijklivehook_read_close,
    .priv_class     = &ijklivehook_class,
};
