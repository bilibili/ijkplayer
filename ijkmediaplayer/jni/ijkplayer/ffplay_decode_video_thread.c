/*****************************************************************************
 * ffplay_video_thread.c
 *****************************************************************************
 *
 * copyright (c) 2001 Fabrice Bellard
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ffplayer.h"

/* ffplay.c 1504 */
static void duplicate_right_border_pixels(SDL_Picture *bmp) {
    int i, width, height;
    Uint8 *p, *maxp;
    for (i = 0; i < 3; i++) {
        width  = bmp->w;
        height = bmp->h;
        if (i > 0) {
            width  >>= 1;
            height >>= 1;
        }
        if (bmp->pitches[i] > width) {
            maxp = bmp->pixels[i] + bmp->pitches[i] * height - 1;
            for (p = bmp->pixels[i] + width - 1; p < maxp; p += bmp->pitches[i])
                *(p+1) = *p;
        }
    }
}
/* ffplay.c 1504 */

/* ffplay.c 1522 */
static int queue_picture(FFPlayer *ffp, AVFrame *src_frame, double pts, int64_t pos, int serial)
{
    VideoState *is = &ffp->is;
    VideoPicture *vp;

#if defined(DEBUG_SYNC) && 0
    printf("frame_type=%c pts=%0.3f\n",
           av_get_picture_type_char(src_frame->pict_type), pts);
#endif

    /* wait until we have space to put a new picture */
    SDL_LockMutex(is->pictq_mutex);

    /* keep the last already displayed picture in the queue */
    while (is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE - 2 &&
           !is->videoq.abort_request) {
        SDL_CondWait(is->pictq_cond, is->pictq_mutex);
    }
    SDL_UnlockMutex(is->pictq_mutex);

    if (is->videoq.abort_request)
        return -1;

    vp = &is->pictq[is->pictq_windex];

#if CONFIG_AVFILTER
    vp->sample_aspect_ratio = ((AVFilterBufferRef *)src_frame->opaque)->video->sample_aspect_ratio;
#else
    vp->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, src_frame);
#endif

    /* alloc or resize hardware picture buffer */
    if (!vp->bmp || vp->reallocate || !vp->allocated ||
        vp->width  != src_frame->width ||
        vp->height != src_frame->height) {
        SDL_Event event;

        vp->allocated  = 0;
        vp->reallocate = 0;
        vp->width = src_frame->width;
        vp->height = src_frame->height;

        /* the allocation must be done in the main thread to avoid
           locking problems. */
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);

        /* wait until the picture is allocated */
        SDL_LockMutex(is->pictq_mutex);
        while (!vp->allocated && !is->videoq.abort_request) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
        /* if the queue is aborted, we have to pop the pending ALLOC event or wait for the allocation to complete */
        if (is->videoq.abort_request && SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_USEREVENT, SDL_LASTEVENT) != 1) {
            while (!vp->allocated) {
                SDL_CondWait(is->pictq_cond, is->pictq_mutex);
            }
        }
        SDL_UnlockMutex(is->pictq_mutex);

        if (is->videoq.abort_request)
            return -1;
    }

    /* if the frame is not skipped, then display it */
    if (vp->bmp) {
        AVPicture pict = { { 0 } };
#if CONFIG_AVFILTER
        avfilter_unref_bufferp(&vp->picref);
        vp->picref = src_frame->opaque;
#endif

        /* get a pointer on the bitmap */
        SDL_LockYUVOverlay (vp->bmp);

        pict.data[0] = vp->bmp->pixels[0];
        pict.data[1] = vp->bmp->pixels[2];
        pict.data[2] = vp->bmp->pixels[1];

        pict.linesize[0] = vp->bmp->pitches[0];
        pict.linesize[1] = vp->bmp->pitches[2];
        pict.linesize[2] = vp->bmp->pitches[1];

#if CONFIG_AVFILTER
        // FIXME use direct rendering
        av_picture_copy(&pict, (AVPicture *)src_frame,
                        src_frame->format, vp->width, vp->height);
#else
        av_opt_get_int(ffp->sws_opts, "sws_flags", 0, &ffp->sws_flags);
        is->img_convert_ctx = sws_getCachedContext(is->img_convert_ctx,
            vp->width, vp->height, src_frame->format, vp->width, vp->height,
            AV_PIX_FMT_YUV420P, ffp->sws_flags, NULL, NULL, NULL);
        if (is->img_convert_ctx == NULL) {
            fprintf(stderr, "Cannot initialize the conversion context\n");
            exit(1);
        }
        sws_scale(is->img_convert_ctx, (const uint8_t *const*)src_frame->data, src_frame->linesize,
                  0, vp->height, pict.data, pict.linesize);
#endif
        /* workaround SDL PITCH_WORKAROUND */
        duplicate_right_border_pixels(vp->bmp);
        /* update the bitmap content */
        SDL_UnlockYUVOverlay(vp->bmp);

        vp->pts = pts;
        vp->pos = pos;
        vp->serial = serial;

        /* now we can update the picture count */
        if (++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE)
            is->pictq_windex = 0;
        SDL_LockMutex(is->pictq_mutex);
        is->pictq_size++;
        SDL_UnlockMutex(is->pictq_mutex);
    }
    return 0;
}
/* ffplay.c 1522 */

/* ffplay.c 1640 */
static int get_video_frame(FFPlayer *ffp, AVFrame *frame, int64_t *pts, AVPacket *pkt, int *serial)
{
    VideoState *is = &ffp->is;
    int got_picture;

    if (packet_queue_get(&is->videoq, pkt, 1, serial) < 0)
        return -1;

    if (pkt->data == flush_pkt.data) {
        avcodec_flush_buffers(is->video_st->codec);

        SDL_LockMutex(is->pictq_mutex);
        // Make sure there are no long delay timers (ideally we should just flush the queue but that's harder)
        while (is->pictq_size && !is->videoq.abort_request) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
        is->video_current_pos = -1;
        is->frame_last_pts = AV_NOPTS_VALUE;
        is->frame_last_duration = 0;
        is->frame_timer = (double)av_gettime() / 1000000.0;
        is->frame_last_dropped_pts = AV_NOPTS_VALUE;
        SDL_UnlockMutex(is->pictq_mutex);

        return 0;
    }

    if(avcodec_decode_video2(is->video_st->codec, frame, &got_picture, pkt) < 0)
        return 0;

    if (got_picture) {
        int ret = 1;

        if (ffp->decoder_reorder_pts == -1) {
            *pts = av_frame_get_best_effort_timestamp(frame);
        } else if (ffp->decoder_reorder_pts) {
            *pts = frame->pkt_pts;
        } else {
            *pts = frame->pkt_dts;
        }

        if (*pts == AV_NOPTS_VALUE) {
            *pts = 0;
        }

        if (ffp->framedrop>0 || (ffp->framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
            SDL_LockMutex(is->pictq_mutex);
            if (is->frame_last_pts != AV_NOPTS_VALUE && *pts) {
                double clockdiff = get_video_clock(is) - get_master_clock(is);
                double dpts = av_q2d(is->video_st->time_base) * *pts;
                double ptsdiff = dpts - is->frame_last_pts;
                if (!isnan(clockdiff) && fabs(clockdiff) < AV_NOSYNC_THRESHOLD &&
                     ptsdiff > 0 && ptsdiff < AV_NOSYNC_THRESHOLD &&
                     clockdiff + ptsdiff - is->frame_last_filter_delay < 0) {
                    is->frame_last_dropped_pos = pkt->pos;
                    is->frame_last_dropped_pts = dpts;
                    is->frame_drops_early++;
                    ret = 0;
                }
            }
            SDL_UnlockMutex(is->pictq_mutex);
        }

        return ret;
    }
    return 0;
}
/* ffplay.c 1640 */

int ijkff_decode_video_thread(void *arg)
{
    FFPlayer *ffp = arg;
    AVPacket pkt = { 0 };
    VideoState *is = &ffp->is;
    AVFrame *frame = avcodec_alloc_frame();
    int64_t pts_int = AV_NOPTS_VALUE;
#if CONFIG_AVFILTER
    int64_t pos = -1;
#endif
    double pts;
    int ret;
    int serial = 0;

#if CONFIG_AVFILTER
    AVCodecContext *codec = is->video_st->codec;
    AVFilterGraph *graph = avfilter_graph_alloc();
    AVFilterContext *filt_out = NULL, *filt_in = NULL;
    int last_w = 0;
    int last_h = 0;
    enum AVPixelFormat last_format = -2;
    int last_serial = -1;

    if (codec->codec->capabilities & CODEC_CAP_DR1) {
        is->use_dr1 = 1;
        codec->get_buffer     = codec_get_buffer;
        codec->release_buffer = codec_release_buffer;
        codec->opaque         = &is->buffer_pool;
    }
#endif

    for (;;) {
#if CONFIG_AVFILTER
        AVFilterBufferRef *picref;
        AVRational tb;
#endif
        while (is->paused && !is->videoq.abort_request)
            SDL_Delay(10);

        avcodec_get_frame_defaults(frame);
        av_free_packet(&pkt);

        ret = get_video_frame(ffp, frame, &pts_int, &pkt, &serial);
        if (ret < 0)
            goto the_end;

        if (!ret)
            continue;

#if CONFIG_AVFILTER
        if (   last_w != frame->width
            || last_h != frame->height
            || last_format != frame->format
            || last_serial != serial) {
            av_log(NULL, AV_LOG_DEBUG,
                   "Video frame changed from size:%dx%d format:%s serial:%d to size:%dx%d format:%s serial:%d\n",
                   last_w, last_h,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(last_format), "none"), last_serial,
                   frame->width, frame->height,
                   (const char *)av_x_if_null(av_get_pix_fmt_name(frame->format), "none"), serial);
            avfilter_graph_free(&graph);
            graph = avfilter_graph_alloc();
            if ((ret = configure_video_filters(graph, is, vfilters, frame)) < 0) {
                SDL_Event event;
                event.type = FF_QUIT_EVENT;
                event.user.data1 = is;
                SDL_PushEvent(&event);
                av_free_packet(&pkt);
                goto the_end;
            }
            filt_in  = is->in_video_filter;
            filt_out = is->out_video_filter;
            last_w = frame->width;
            last_h = frame->height;
            last_format = frame->format;
            last_serial = serial;
        }

        frame->pts = pts_int;
        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);
        if (is->use_dr1 && frame->opaque) {
            FrameBuffer      *buf = frame->opaque;
            AVFilterBufferRef *fb = avfilter_get_video_buffer_ref_from_arrays(
                                        frame->data, frame->linesize,
                                        AV_PERM_READ | AV_PERM_PRESERVE,
                                        frame->width, frame->height,
                                        frame->format);

            avfilter_copy_frame_props(fb, frame);
            fb->buf->priv           = buf;
            fb->buf->free           = filter_release_buffer;

            buf->refcount++;
            av_buffersrc_add_ref(filt_in, fb, AV_BUFFERSRC_FLAG_NO_COPY);

        } else
            av_buffersrc_write_frame(filt_in, frame);

        av_free_packet(&pkt);

        while (ret >= 0) {
            is->frame_last_returned_time = av_gettime() / 1000000.0;

            ret = av_buffersink_get_buffer_ref(filt_out, &picref, 0);
            if (ret < 0) {
                ret = 0;
                break;
            }

            is->frame_last_filter_delay = av_gettime() / 1000000.0 - is->frame_last_returned_time;
            if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0)
                is->frame_last_filter_delay = 0;

            avfilter_copy_buf_props(frame, picref);

            pts_int = picref->pts;
            tb      = filt_out->inputs[0]->time_base;
            pos     = picref->pos;
            frame->opaque = picref;

            if (av_cmp_q(tb, is->video_st->time_base)) {
                av_unused int64_t pts1 = pts_int;
                pts_int = av_rescale_q(pts_int, tb, is->video_st->time_base);
                av_dlog(NULL, "video_thread(): "
                        "tb:%d/%d pts:%"PRId64" -> tb:%d/%d pts:%"PRId64"\n",
                        tb.num, tb.den, pts1,
                        is->video_st->time_base.num, is->video_st->time_base.den, pts_int);
            }
            pts = pts_int * av_q2d(is->video_st->time_base);
            ret = queue_picture(is, frame, pts, pos, serial);
        }
#else
        pts = pts_int * av_q2d(is->video_st->time_base);
        ret = queue_picture(ffp, frame, pts, pkt.pos, serial);
#endif

        if (ret < 0)
            goto the_end;
    }
 the_end:
    avcodec_flush_buffers(is->video_st->codec);
#if CONFIG_AVFILTER
    avfilter_graph_free(&graph);
#endif
    av_free_packet(&pkt);
    avcodec_free_frame(&frame);
    return 0;
}
/* ffplay.c 1522 */
