/*
 * ff_ffplay.h
 *
 * Copyright (c) 2003 Bilibili
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

#ifndef FFPLAY__FF_FFPLAY_H
#define FFPLAY__FF_FFPLAY_H

#include "ff_ffplay_def.h"
#include "ff_fferror.h"
#include "ff_ffmsg.h"

void      ffp_global_init();
void      ffp_global_uninit();
void      ffp_global_set_log_report(int use_report);
void      ffp_global_set_log_level(int log_level);
void      ffp_global_set_inject_callback(ijk_inject_callback cb);
void      ffp_io_stat_register(void (*cb)(const char *url, int type, int bytes));
void      ffp_io_stat_complete_register(void (*cb)(const char *url,
                                                   int64_t read_bytes, int64_t total_size,
                                                   int64_t elpased_time, int64_t total_duration));

FFPlayer *ffp_create();
void      ffp_destroy(FFPlayer *ffp);
void      ffp_destroy_p(FFPlayer **pffp);
void      ffp_reset(FFPlayer *ffp);

/* set options before ffp_prepare_async_l() */

void     ffp_set_frame_at_time(FFPlayer *ffp, const char *path, int64_t start_time, int64_t end_time, int num, int definition);
void     *ffp_set_inject_opaque(FFPlayer *ffp, void *opaque);
void     *ffp_set_ijkio_inject_opaque(FFPlayer *ffp, void *opaque);
void      ffp_set_option(FFPlayer *ffp, int opt_category, const char *name, const char *value);
void      ffp_set_option_int(FFPlayer *ffp, int opt_category, const char *name, int64_t value);

int       ffp_get_video_codec_info(FFPlayer *ffp, char **codec_info);
int       ffp_get_audio_codec_info(FFPlayer *ffp, char **codec_info);

/* playback controll */
int       ffp_prepare_async_l(FFPlayer *ffp, const char *file_name);
int       ffp_start_from_l(FFPlayer *ffp, long msec);
int       ffp_start_l(FFPlayer *ffp);
int       ffp_pause_l(FFPlayer *ffp);
int       ffp_is_paused_l(FFPlayer *ffp);
int       ffp_stop_l(FFPlayer *ffp);
int       ffp_wait_stop_l(FFPlayer *ffp);

/* all in milliseconds */
int       ffp_seek_to_l(FFPlayer *ffp, long msec);
long      ffp_get_current_position_l(FFPlayer *ffp);
long      ffp_get_duration_l(FFPlayer *ffp);
long      ffp_get_playable_duration_l(FFPlayer *ffp);
void      ffp_set_loop(FFPlayer *ffp, int loop);
int       ffp_get_loop(FFPlayer *ffp);

/* for internal usage */
int       ffp_packet_queue_init(PacketQueue *q);
void      ffp_packet_queue_destroy(PacketQueue *q);
void      ffp_packet_queue_abort(PacketQueue *q);
void      ffp_packet_queue_start(PacketQueue *q);
void      ffp_packet_queue_flush(PacketQueue *q);
int       ffp_packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial);
int       ffp_packet_queue_get_or_buffering(FFPlayer *ffp, PacketQueue *q, AVPacket *pkt, int *serial, int *finished);
int       ffp_packet_queue_put(PacketQueue *q, AVPacket *pkt);
bool      ffp_is_flush_packet(AVPacket *pkt);

Frame    *ffp_frame_queue_peek_writable(FrameQueue *f);
void      ffp_frame_queue_push(FrameQueue *f);

int       ffp_queue_picture(FFPlayer *ffp, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial);

int       ffp_get_master_sync_type(VideoState *is);
double    ffp_get_master_clock(VideoState *is);

void      ffp_toggle_buffering_l(FFPlayer *ffp, int start_buffering);
void      ffp_toggle_buffering(FFPlayer *ffp, int start_buffering);
void      ffp_check_buffering_l(FFPlayer *ffp);
void      ffp_track_statistic_l(FFPlayer *ffp, AVStream *st, PacketQueue *q, FFTrackCacheStatistic *cache);
void      ffp_audio_statistic_l(FFPlayer *ffp);
void      ffp_video_statistic_l(FFPlayer *ffp);
void      ffp_statistic_l(FFPlayer *ffp);

int       ffp_video_thread(FFPlayer *ffp);

void      ffp_set_video_codec_info(FFPlayer *ffp, const char *module, const char *codec);
void      ffp_set_audio_codec_info(FFPlayer *ffp, const char *module, const char *codec);
void      ffp_set_subtitle_codec_info(FFPlayer *ffp, const char *module, const char *codec);

void      ffp_set_playback_rate(FFPlayer *ffp, float rate);
void      ffp_set_playback_volume(FFPlayer *ffp, float volume);
int       ffp_get_video_rotate_degrees(FFPlayer *ffp);
int       ffp_set_stream_selected(FFPlayer *ffp, int stream, int selected);

float     ffp_get_property_float(FFPlayer *ffp, int id, float default_value);
void      ffp_set_property_float(FFPlayer *ffp, int id, float value);
int64_t   ffp_get_property_int64(FFPlayer *ffp, int id, int64_t default_value);
void      ffp_set_property_int64(FFPlayer *ffp, int id, int64_t value);

// must be freed with free();
struct IjkMediaMeta *ffp_get_meta_l(FFPlayer *ffp);

#endif
