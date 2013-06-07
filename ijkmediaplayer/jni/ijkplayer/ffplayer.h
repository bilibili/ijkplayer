/*****************************************************************************
 * ffplayer.h
 *****************************************************************************
 *
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

#ifndef IJKPLAYER__FFPLAYER_H
#define IJKPLAYER__FFPLAYER_H

#include <stdint.h>
#include "ffplay.h"

/*----------------------------------------
 *
 */
typedef struct FFPlayer {
    /* ffplay context */
    VideoState      is;

    /* format/codec options */
    AVDictionary   *format_opts;
    AVDictionary   *codec_opts;

    /* ffplay options specified by the user */
    // AVInputFormat  *file_iformat;
    // const char     *input_filename;
    // const char     *window_title;
    // int             fs_screen_width;
    // int             fs_screen_height;
    // int             default_width  = 640;
    // int             default_height = 480;
    // int             screen_width  = 0;
    // int             screen_height = 0;
    int             audio_disable;
    int             video_disable;
    int             subtitle_disable;
    int             wanted_stream[AVMEDIA_TYPE_NB];
    int             seek_by_bytes;
    // int             display_disable;
    int             show_status;
    // int             av_sync_type = AV_SYNC_AUDIO_MASTER;
    int64_t         start_time;
    // int64_t         duration = AV_NOPTS_VALUE;
    // int             workaround_bugs = 1;
    // int             fast = 0;
    int             genpts;
    // int             lowres = 0;
    // int             idct = FF_IDCT_AUTO;
    // enum AVDiscard  skip_frame       = AVDISCARD_DEFAULT;
    // enum AVDiscard  skip_idct        = AVDISCARD_DEFAULT;
    // enum AVDiscard  skip_loop_filter = AVDISCARD_DEFAULT;
    // int             error_concealment = 3;
    // int             decoder_reorder_pts = -1;
    // int             autoexit;
    // int             exit_on_keydown;
    // int             exit_on_mousedown;
    // int             loop = 1;
    // int             framedrop = -1;
    int             infinite_buffer;
    enum ShowMode   show_mode;
    // const char     *audio_codec_name;
    // const char     *subtitle_codec_name;
    // const char     *video_codec_name;
    // double          rdftspeed = 0.02;
    // int64_t         cursor_last_shown;
    // int             cursor_hidden = 0;
    #if CONFIG_AVFILTER
    // char            *vfilters = NULL;
    #endif

    /* callback */
    int             (*decode_interrupt_cb)(void *ctx);
} FFPlayer;

inline static void ijkff_reset(FFPlayer *ffp)
{
    /* FIXME: ffplay context reset */

    /* format/codec options */
    av_dict_free(&ffp->format_opts);
    av_dict_free(&ffp->codec_opts);

    /* ffplay options specified by the user */
    ffp->audio_disable          = 0;
    ffp->video_disable          = 0;
    ffp->subtitle_disable       = 0;
    ffp->wanted_stream[AVMEDIA_TYPE_AUDIO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_VIDEO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_SUBTITLE]   = -1;
    ffp->seek_by_bytes          = -1;
    ffp->show_status            = 1;
    ffp->start_time             = AV_NOPTS_VALUE;
    ffp->genpts                 = 0;
    ffp->infinite_buffer        = -1;
    ffp->show_mode              = SHOW_MODE_NONE;

    /* callback */
    ffp->decode_interrupt_cb    = NULL;
}

void ijkff_global_init();
void ijkff_global_uninit();

/*
 * ffmpeg api listed below must be locked
 *
 *  av_set_cpu_flags_mask();
 *  av_register_all();
 *  avcodec_register_all();
 *
 *  avcodec_open
 *  avcodec_open2
 *  avcodec_close
 *
 *  avformat_find_stream_info
 *  av_find_stream_info
 */
void ijkff_avcodec_lock();
void ijkff_avcodec_unlock();

#endif

