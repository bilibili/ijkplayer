/*****************************************************************************
 * ffplay_clock.h
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

#ifndef IJKPLAYER__FFPLAY_CLOCK_H
#define IJKPLAYER__FFPLAY_CLOCK_H

#include "ffplay_def.h"

/* get the current audio clock value */
inline static double get_audio_clock(VideoState *is)
{
    if (is->audio_clock_serial != is->audioq.serial)
        return NAN;
    if (is->paused) {
        return is->audio_current_pts;
    } else {
        return is->audio_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current video clock value */
inline static double get_video_clock(VideoState *is)
{
    if (is->video_clock_serial != is->videoq.serial)
        return NAN;
    if (is->paused) {
        return is->video_current_pts;
    } else {
        return is->video_current_pts_drift + av_gettime() / 1000000.0;
    }
}

/* get the current external clock value */
inline static double get_external_clock(VideoState *is)
{
    if (is->paused) {
        return is->external_clock;
    } else {
        double time = av_gettime() / 1000000.0;
        return is->external_clock_drift + time - (time - is->external_clock_time / 1000000.0) * (1.0 - is->external_clock_speed);
    }
}

inline static int get_master_sync_type(VideoState *is) {
    if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        if (is->video_st)
            return AV_SYNC_VIDEO_MASTER;
        else
            return AV_SYNC_AUDIO_MASTER;
    } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
        if (is->audio_st)
            return AV_SYNC_AUDIO_MASTER;
        else
            return AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return AV_SYNC_EXTERNAL_CLOCK;
    }
}

/* get the current master clock value */
inline static double get_master_clock(VideoState *is)
{
    double val;

    switch (get_master_sync_type(is)) {
        case AV_SYNC_VIDEO_MASTER:
            val = get_video_clock(is);
            break;
        case AV_SYNC_AUDIO_MASTER:
            val = get_audio_clock(is);
            break;
        default:
            val = get_external_clock(is);
            break;
    }
    return val;
}

inline static void update_external_clock_pts(VideoState *is, double pts)
{
   is->external_clock_time = av_gettime();
   is->external_clock = pts;
   is->external_clock_drift = pts - is->external_clock_time / 1000000.0;
}

inline static void check_external_clock_sync(VideoState *is, double pts) {
    double ext_clock = get_external_clock(is);
    if (isnan(ext_clock) || fabs(ext_clock - pts) > AV_NOSYNC_THRESHOLD) {
        update_external_clock_pts(is, pts);
    }
}

#endif
