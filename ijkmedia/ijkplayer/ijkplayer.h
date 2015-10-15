/*
 * ijkplayer.h
 *
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

#ifndef IJKPLAYER_ANDROID__IJKPLAYER_H
#define IJKPLAYER_ANDROID__IJKPLAYER_H

#include <stdbool.h>
#include "ff_ffmsg_queue.h"

#include "ijkmeta.h"

#ifndef MPTRACE
#define MPTRACE ALOGD
#endif

typedef struct IjkMediaPlayer IjkMediaPlayer;
struct FFPlayer;
struct SDL_Vout;

/*-
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_IDLE);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_INITIALIZED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ASYNC_PREPARING);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PREPARED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STARTED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_PAUSED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_COMPLETED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_STOPPED);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_ERROR);
 MPST_CHECK_NOT_RET(mp->mp_state, MP_STATE_END);
 */

/*-
 * ijkmp_set_data_source()  -> MP_STATE_INITIALIZED
 *
 * ijkmp_reset              -> self
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_IDLE               0

/*-
 * ijkmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_INITIALIZED        1

/*-
 *                   ...    -> MP_STATE_PREPARED
 *                   ...    -> MP_STATE_ERROR
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_ASYNC_PREPARING    2

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_PREPARED           3

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> self
 * ijkmp_pause()            -> MP_STATE_PAUSED
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *                   ...    -> MP_STATE_COMPLETED
 *                   ...    -> MP_STATE_ERROR
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_STARTED            4

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED
 * ijkmp_pause()            -> self
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_PAUSED             5

/*-
 * ijkmp_seek_to()          -> self
 * ijkmp_start()            -> MP_STATE_STARTED (from beginning)
 * ijkmp_pause()            -> self
 * ijkmp_stop()             -> MP_STATE_STOPPED
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_COMPLETED          6

/*-
 * ijkmp_stop()             -> self
 * ijkmp_prepare_async()    -> MP_STATE_ASYNC_PREPARING
 *
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_STOPPED            7

/*-
 * ijkmp_reset              -> MP_STATE_IDLE
 * ijkmp_release            -> MP_STATE_END
 */
#define MP_STATE_ERROR              8

/*-
 * ijkmp_release            -> self
 */
#define MP_STATE_END                9



#define IJKMP_IO_STAT_READ 1


#define IJKMP_OPT_CATEGORY_FORMAT FFP_OPT_CATEGORY_FORMAT
#define IJKMP_OPT_CATEGORY_CODEC  FFP_OPT_CATEGORY_CODEC
#define IJKMP_OPT_CATEGORY_SWS    FFP_OPT_CATEGORY_SWS
#define IJKMP_OPT_CATEGORY_PLAYER FFP_OPT_CATEGORY_PLAYER


void            ijkmp_global_init();
void            ijkmp_global_uninit();
void            ijkmp_global_set_log_report(int use_report);
void            ijkmp_global_set_log_level(int log_level);   // log_level = AV_LOG_xxx
void            ijkmp_global_set_inject_callback(ijk_inject_callback cb);
const char     *ijkmp_version_ident();
unsigned int    ijkmp_version_int();
void            ijkmp_io_stat_register(void (*cb)(const char *url, int type, int bytes));
void            ijkmp_io_stat_complete_register(void (*cb)(const char *url,
                                                           int64_t read_bytes, int64_t total_size,
                                                           int64_t elpased_time, int64_t total_duration));

// ref_count is 1 after open
IjkMediaPlayer *ijkmp_create(int (*msg_loop)(void*));
void            ijkmp_set_inject_opaque(IjkMediaPlayer *mp, void *opaque);

void            ijkmp_set_option(IjkMediaPlayer *mp, int opt_category, const char *name, const char *value);
void            ijkmp_set_option_int(IjkMediaPlayer *mp, int opt_category, const char *name, int64_t value);

int             ijkmp_get_video_codec_info(IjkMediaPlayer *mp, char **codec_info);
int             ijkmp_get_audio_codec_info(IjkMediaPlayer *mp, char **codec_info);
void            ijkmp_set_playback_rate(IjkMediaPlayer *mp, float rate);

float           ijkmp_get_property_float(IjkMediaPlayer *mp, int id, float default_value);
void            ijkmp_set_property_float(IjkMediaPlayer *mp, int id, float default_value);

// must be freed with free();
IjkMediaMeta   *ijkmp_get_meta_l(IjkMediaPlayer *mp);

// preferred to be called explicity, can be called multiple times
// NOTE: ijkmp_shutdown may block thread
void            ijkmp_shutdown(IjkMediaPlayer *mp);

void            ijkmp_inc_ref(IjkMediaPlayer *mp);

// call close at last release, also free memory
// NOTE: ijkmp_dec_ref may block thread
void            ijkmp_dec_ref(IjkMediaPlayer *mp);
void            ijkmp_dec_ref_p(IjkMediaPlayer **pmp);

int             ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url);
int             ijkmp_prepare_async(IjkMediaPlayer *mp);
int             ijkmp_start(IjkMediaPlayer *mp);
int             ijkmp_pause(IjkMediaPlayer *mp);
int             ijkmp_stop(IjkMediaPlayer *mp);
int             ijkmp_seek_to(IjkMediaPlayer *mp, long msec);
int             ijkmp_get_state(IjkMediaPlayer *mp);
bool            ijkmp_is_playing(IjkMediaPlayer *mp);
long            ijkmp_get_current_position(IjkMediaPlayer *mp);
long            ijkmp_get_duration(IjkMediaPlayer *mp);
long            ijkmp_get_playable_duration(IjkMediaPlayer *mp);
void            ijkmp_set_loop(IjkMediaPlayer *mp, int loop);
int             ijkmp_get_loop(IjkMediaPlayer *mp);

void           *ijkmp_get_weak_thiz(IjkMediaPlayer *mp);
void           *ijkmp_set_weak_thiz(IjkMediaPlayer *mp, void *weak_thiz);

/* return < 0 if aborted, 0 if no packet and > 0 if packet.  */
int             ijkmp_get_msg(IjkMediaPlayer *mp, AVMessage *msg, int block);

#endif
