/*
 * ff_ffplaye_def.h
 *
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKPLAYER__FF_FFPLAY_DEF_H
#define IJKPLAYER__FF_FFPLAY_DEF_H

#include "ff_ffinc.h"
#include "ff_ffplay_config.h"

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 5

/* SDL audio buffer size, in samples. Should be small to have precise
   A/V sync as SDL does not have hardware buffer fullness info. */
#define SDL_AUDIO_BUFFER_SIZE 1024

/* no AV sync correction is done if below the AV sync threshold */
#define AV_SYNC_THRESHOLD 0.01
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#ifdef IJK_FFPLAY_MERGE
#define CURSOR_HIDE_DELAY 1000000

static int64_t sws_flags = SWS_BICUBIC;
#endif

typedef struct MyAVPacketList {
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    MyAVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 4
#define SUBPICTURE_QUEUE_SIZE 4

typedef struct VideoPicture {
    double pts;             // presentation timestamp for this picture
    int64_t pos;            // byte position in file
    SDL_VoutOverlay *bmp;
    int width, height; /* source height & width */
    AVRational sample_aspect_ratio;
    int allocated;
    int reallocate;
    int serial;

#if CONFIG_AVFILTER
    AVFilterBufferRef *picref;
#endif
} VideoPicture;

typedef struct SubPicture {
    double pts; /* presentation time stamp for this picture */
    AVSubtitle sub;
} SubPicture;

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
} AudioParams;

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef struct VideoState {
    SDL_Thread *read_tid;
    SDL_Thread _read_tid;
    SDL_Thread *video_tid;
    SDL_Thread _video_tid;
    AVInputFormat *iformat;
    int no_background;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;

    int audio_stream;

    int av_sync_type;
    double external_clock;                   ///< external clock base
    double external_clock_drift;             ///< external clock base - time (av_gettime) at which we updated external_clock
    int64_t external_clock_time;             ///< last reference time
    double external_clock_speed;             ///< speed of the external clock

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t silence_buf[SDL_AUDIO_BUFFER_SIZE];
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    AVPacket audio_pkt_temp;
    AVPacket audio_pkt;
    int audio_pkt_temp_serial;
    struct AudioParams audio_src;
    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    double audio_current_pts;
    double audio_current_pts_drift;
    int frame_drops_early;
    int frame_drops_late;
    AVFrame *frame;

    enum ShowMode {
        SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
    } show_mode;
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;
    double last_vis_time;

#ifdef IJK_FFPLAY_MERGE
    SDL_Thread *subtitle_tid;
    int subtitle_stream;
    int subtitle_stream_changed;
    AVStream *subtitle_st;
    PacketQueue subtitleq;
    SubPicture subpq[SUBPICTURE_QUEUE_SIZE];
    int subpq_size, subpq_rindex, subpq_windex;
    SDL_mutex *subpq_mutex;
    SDL_cond *subpq_cond;
#endif

    double frame_timer;
    double frame_last_pts;
    double frame_last_duration;
    double frame_last_dropped_pts;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int64_t frame_last_dropped_pos;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double video_current_pts;       // current displayed pts
    double video_current_pts_drift; // video_current_pts - time (av_gettime) at which we updated video_current_pts - used to have running video pts
    int64_t video_current_pos;      // current displayed file pos
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    int video_clock_serial;
    VideoPicture pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int pictq_size, pictq_rindex, pictq_windex;
    SDL_mutex *pictq_mutex;
    SDL_cond *pictq_cond;
#if !CONFIG_AVFILTER
    struct SwsContext *img_convert_ctx;
#endif
#ifdef IJK_FFPLAY_MERGE
    SDL_Rect last_display_rect;
#endif

    char filename[1024];
    int width, height, xleft, ytop;
    int step;

#if CONFIG_AVFILTER
    AVFilterContext *in_video_filter;   // the first filter in the video chain
    AVFilterContext *out_video_filter;  // the last filter in the video chain
    int use_dr1;
    FrameBuffer *buffer_pool;
#endif

    int last_video_stream, last_audio_stream, last_subtitle_stream;

    SDL_cond *continue_read_thread;

    SDL_Thread *video_refresh_tid;
    SDL_Thread _video_refresh_tid;
} VideoState;

/* options specified by the user */
#ifdef IJK_FFPLAY_MERGE
static AVInputFormat *file_iformat;
static const char *input_filename;
static const char *window_title;
static int fs_screen_width;
static int fs_screen_height;
static int default_width  = 640;
static int default_height = 480;
static int screen_width  = 0;
static int screen_height = 0;
static int audio_disable;
static int video_disable;
static int subtitle_disable;
static int wanted_stream[AVMEDIA_TYPE_NB] = {
    [AVMEDIA_TYPE_AUDIO]    = -1,
    [AVMEDIA_TYPE_VIDEO]    = -1,
    [AVMEDIA_TYPE_SUBTITLE] = -1,
};
static int seek_by_bytes = -1;
static int display_disable;
static int show_status = 1;
static int av_sync_type = AV_SYNC_AUDIO_MASTER;
static int64_t start_time = AV_NOPTS_VALUE;
static int64_t duration = AV_NOPTS_VALUE;
static int workaround_bugs = 1;
static int fast = 0;
static int genpts = 0;
static int lowres = 0;
static int idct = FF_IDCT_AUTO;
static enum AVDiscard skip_frame       = AVDISCARD_DEFAULT;
static enum AVDiscard skip_idct        = AVDISCARD_DEFAULT;
static enum AVDiscard skip_loop_filter = AVDISCARD_DEFAULT;
static int error_concealment = 3;
static int decoder_reorder_pts = -1;
static int autoexit;
static int exit_on_keydown;
static int exit_on_mousedown;
static int loop = 1;
static int framedrop = -1;
static int infinite_buffer = -1;
static enum ShowMode show_mode = SHOW_MODE_NONE;
static const char *audio_codec_name;
static const char *subtitle_codec_name;
static const char *video_codec_name;
double rdftspeed = 0.02;
static int64_t cursor_last_shown;
static int cursor_hidden = 0;
#if CONFIG_AVFILTER
static char *vfilters = NULL;
#endif

/* current context */
static int is_full_screen;
static int64_t audio_callback_time;

static AVPacket flush_pkt;
#endif

extern AVPacket flush_pkt;

#ifdef IJK_FFPLAY_MERGE
#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

static SDL_Surface *screen;
#endif

/*****************************************************************************
 * end at line 330 in ffplay.c
 * near packet_queue_put
 ****************************************************************************/

/* ffplayer */
typedef struct FFPlayer {
    /* ffplay context */
    VideoState *is;

    /* format/codec options */
    AVDictionary *format_opts;
    AVDictionary *codec_opts;
    AVDictionary *sws_opts;

    /* ffplay options specified by the user */
#ifdef IJK_FFPLAY_MERGE
    AVInputFormat *file_iformat;
#endif
    char *input_filename;
#ifdef IJK_FFPLAY_MERGE
    const char *window_title;
    int fs_screen_width;
    int fs_screen_height;
#endif
    int default_width;
    int default_height;
#ifdef IJK_FFPLAY_MERGE
    int screen_width = 0;
    int screen_height = 0;
#endif
    int audio_disable;
    int video_disable;
    int subtitle_disable;
    int wanted_stream[AVMEDIA_TYPE_NB];
    int seek_by_bytes;
    int display_disable;
    int show_status;
    int av_sync_type;
    int64_t start_time;
    int64_t duration;
    int workaround_bugs;
    int fast;
    int genpts;
    int lowres;
    int idct;
    enum AVDiscard skip_frame;
    enum AVDiscard skip_idct;
    enum AVDiscard skip_loop_filter;
    int error_concealment;
    int decoder_reorder_pts;
    int autoexit;
#ifdef IJK_FFPLAY_MERGE
    int exit_on_keydown;
    int exit_on_mousedown;
#endif
    int loop;
    int framedrop;
    int infinite_buffer;
    enum ShowMode show_mode;
    char *audio_codec_name;
    char *subtitle_codec_name;
    char *video_codec_name;
    double rdftspeed;
#ifdef IJK_FFPLAY_MERGE
    int64_t cursor_last_shown;
    int cursor_hidden = 0;
#endif
#if CONFIG_AVFILTER
    char *vfilters;
#endif

    int64_t sws_flags;

    /* current context */
#ifdef IJK_FFPLAY_MERGE
    int is_full_screen;
#endif
    int64_t audio_callback_time;
#ifdef IJK_FFPLAY_MERGE
    SDL_Surface *screen;
#endif

    /* extra fields */
    SDL_Aout *aout;
    SDL_Vout *vout;
    int sar_num;
    int sar_den;

    int last_error;

    void  *msg_opaque;
    void (*msg_handler)(void *opaque, int what, int arg1, int arg2, void *data);
} FFPlayer;

#define IJKFF_SAFE_FREE(p) do {free(p); p = NULL;} while(0)
#define fftime_to_milliseconds(ts) (ts / (AV_TIME_BASE / 1000))
#define milliseconds_to_fftime(ms) (ms * (AV_TIME_BASE / 1000))

inline static void ijkff_reset_internal(FFPlayer *ffp)
{
    /* ffp->is closed in stream_close() */

    /* format/codec options */
    av_dict_free(&ffp->format_opts);
    av_dict_free(&ffp->codec_opts);
    av_dict_free(&ffp->sws_opts);

    /* ffplay options specified by the user */
    IJKFF_SAFE_FREE(ffp->input_filename);
    ffp->default_width          = 640;
    ffp->default_height         = 480;
    ffp->audio_disable          = 0;
    ffp->video_disable          = 0;
    ffp->subtitle_disable       = 0;
    ffp->wanted_stream[AVMEDIA_TYPE_AUDIO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_VIDEO]      = -1;
    ffp->wanted_stream[AVMEDIA_TYPE_SUBTITLE]   = -1;
    ffp->seek_by_bytes          = -1;
    ffp->display_disable        = 0;
    ffp->show_status            = 1;
    ffp->av_sync_type           = AV_SYNC_AUDIO_MASTER;
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
    ffp->decoder_reorder_pts    = -1;
    ffp->autoexit               = 0;
    ffp->loop                   = 1;
    ffp->framedrop              = -1;
    ffp->infinite_buffer        = -1;
    ffp->show_mode              = SHOW_MODE_NONE;
    IJKFF_SAFE_FREE(ffp->audio_codec_name);
    IJKFF_SAFE_FREE(ffp->subtitle_codec_name);
    IJKFF_SAFE_FREE(ffp->video_codec_name);
    ffp->rdftspeed              = 0.02;
#if CONFIG_AVFILTER
    ffp->vfilters               = NULL;
#endif

    ffp->sws_flags              = SWS_BICUBIC;

    /* current context */
    ffp->audio_callback_time    = 0;

    /* extra fields */
    ffp->aout                   = NULL; /* reset outside */
    ffp->vout                   = NULL; /* reset outside */
    ffp->sar_num                = 0;
    ffp->sar_den                = 0;

    ffp->last_error             = 0;

    ffp->msg_opaque             = 0;
    ffp->msg_handler            = NULL;
}

inline static void ijkff_notify_msg(FFPlayer *ffp, int what, int arg1, int arg2, void* data) {
    if (ffp->msg_handler)
        ffp->msg_handler(ffp->msg_opaque, what, arg1, arg2, data);
}

#endif
