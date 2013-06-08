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
#if 0
    AVInputFormat  *file_iformat;
#endif
    char           *input_filename;
#if 0
    const char     *window_title;
    int             fs_screen_width;
    int             fs_screen_height;
    int             default_width  = 640;
    int             default_height = 480;
    int             screen_width  = 0;
    int             screen_height = 0;
#endif
    int             audio_disable;
    int             video_disable;
    int             subtitle_disable;
    int             wanted_stream[AVMEDIA_TYPE_NB];
    int             seek_by_bytes;
#if 0
    int             display_disable;
#endif
    int             show_status;
#if 0
    int             av_sync_type = AV_SYNC_AUDIO_MASTER;
#endif
    int64_t         start_time;
    int64_t         duration;
    int             workaround_bugs;
    int             fast;
    int             genpts;
    int             lowres;
    int             idct;
    enum AVDiscard  skip_frame;
    enum AVDiscard  skip_idct;
    enum AVDiscard  skip_loop_filter;
    int             error_concealment;
#if 0
    int             decoder_reorder_pts = -1;
#endif
    int             autoexit;
#if 0
    int             exit_on_keydown;
    int             exit_on_mousedown;
#endif
    int             loop;
#if 0
    int             framedrop = -1;
#endif
    int             infinite_buffer;
    enum ShowMode   show_mode;
    char           *audio_codec_name;
    char           *subtitle_codec_name;
    char           *video_codec_name;
#if 0
    double          rdftspeed = 0.02;
    int64_t         cursor_last_shown;
    int             cursor_hidden = 0;
#endif
#if CONFIG_AVFILTER
    char           *vfilters;
#endif

    /* current context */
#if 0
    int             is_full_screen;
#endif
    int64_t         audio_callback_time;

    /* callback */
    int           (*decode_interrupt_cb)(void *ctx);
} FFPlayer;

#define IJKFF_SAFE_FREE(p) do {free(p); p = NULL;} while(0)

inline static void ijkff_reset(FFPlayer *ffp)
{
    /* FIXME: reset ffp->is */

    /* format/codec options */
    av_dict_free(&ffp->format_opts);
    av_dict_free(&ffp->codec_opts);

    /* ffplay options specified by the user */
    IJKFF_SAFE_FREE(ffp->input_filename);
    ffp->audio_disable          = 0;
    ffp->video_disable          = 0;
    ffp->subtitle_disable       = 0;
    ffp->wanted_stream[AVMEDIA_TYPE_AUDIO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_VIDEO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_SUBTITLE]   = -1;
    ffp->seek_by_bytes          = -1;
    ffp->show_status            = 1;
    ffp->start_time             = AV_NOPTS_VALUE;
    ffp->duration               = AV_NOPTS_VALUE;
    ffp->workaround_bugs        = 1;
    ffp->fast                   = 0;
    ffp->genpts                 = 0;
    ffp->lowres                 = 0;
    ffp->idct                   = FF_IDCT_AUTO;
    ffp->skip_frame             = AVDISCARD_DEFAULT;
    ffp->skip_idct              = AVDISCARD_DEFAULT;
    ffp->skip_loop_filter       = AVDISCARD_DEFAULT;
    ffp->error_concealment      = 3;
    ffp->autoexit               = 0;
    ffp->loop                   = 1;
    ffp->infinite_buffer        = -1;
    ffp->show_mode              = SHOW_MODE_NONE;
    IJKFF_SAFE_FREE(ffp->audio_codec_name);
    IJKFF_SAFE_FREE(ffp->subtitle_codec_name);
    IJKFF_SAFE_FREE(ffp->video_codec_name);
#if CONFIG_AVFILTER
    ffp->vfilters               = NULL;
#endif

    /* current context */
    ffp->audio_callback_time    = 0;

    /* callback */
    ffp->decode_interrupt_cb    = NULL;
}

void ijkff_global_init();
void ijkff_global_uninit();

#endif

