/*****************************************************************************
* ijkplayer_desktop.c
*****************************************************************************
*
* copyright (c) 2019 befovy <befovy@gmail.com>
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


#include <inttypes.h>

#include "ijkplayer_desktop.h"
#include "ijkplayer/desktop/pipeline_desktop.h"
#include "ijkplayer/ff_ffmsg_queue.h"


struct IjkFFMediaPlayer {
    IjkMediaPlayer *mp;
    void *event_cb_data;
    ijkff_event_cb event_cb;

    IjkFFOverlay overlay;
    void *overlay_cb_data;
    ijkff_overlay_cb overlay_cb;
};

static int ijkff_msg_loop(void *arg)
{
    IjkMediaPlayer *mp = arg;
    IjkFFMediaPlayer *fp = ijkmp_set_weak_thiz(mp, NULL);

    while (true) {
        AVMessage msg;

        int retval = ijkmp_get_msg(mp, &msg, 1);
        if (retval < 0)
            break;

        if (fp->event_cb) {
            fp->event_cb(fp->event_cb_data, msg.what, msg.arg1, msg.arg2, msg.obj);
        }
    }

    ijkmp_dec_ref_p(&mp);
    return 0;
}

IjkFFMediaPlayer *ijkff_create()
{
    ijkmp_global_init();
    IjkFFMediaPlayer *fp = mallocz(sizeof(IjkFFMediaPlayer));
    fp->mp = ijkmp_desktop_create(ijkff_msg_loop);

    ijkmp_set_weak_thiz(fp->mp, fp);
    ijkmp_set_inject_opaque(fp->mp, fp);
    return fp;
}


int ijkff_set_data_source(IjkFFMediaPlayer *fp, const char *url)
{
    assert(fp);
    return ijkmp_set_data_source(fp->mp, url);
}

int ijkff_prepare_async(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_prepare_async(fp->mp);
}

int ijkff_start(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_start(fp->mp);
}

int ijkff_stop(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_stop(fp->mp);
}

int ijkff_pause(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_pause(fp->mp);
}

int ijkff_reset(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_reset(fp->mp);
}

bool ijkff_is_playing(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_is_playing(fp->mp);
}

void ijkff_shutdown(IjkFFMediaPlayer *fp)
{
    assert(fp);
    if (fp->mp) {
        ijkmp_shutdown(fp->mp);
        ijkmp_set_inject_opaque(fp->mp, NULL);
        ijkmp_set_ijkio_inject_opaque(fp->mp, NULL);
        ijkmp_dec_ref(fp->mp);
        fp->mp = NULL;
        fp->event_cb = NULL;
        fp->event_cb_data = NULL;
    }
}

int64_t ijkff_get_current_position(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_get_current_position(fp->mp);
}

int64_t ijkff_get_duration(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_get_duration(fp->mp);
}

int ijkff_seek_to(IjkFFMediaPlayer *fp, int64_t msec)
{
    assert(fp);
    return ijkmp_seek_to(fp->mp, msec);
}

void ijkff_set_loop(IjkFFMediaPlayer *fp, int loop)
{
    assert(fp);
    ijkmp_set_loop(fp->mp, loop);
}

int ijkff_get_loop(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_get_loop(fp->mp);
}


void ijkff_set_speed(IjkFFMediaPlayer *fp, float speed)
{
    assert(fp);
    ijkmp_set_playback_rate(fp->mp, speed);
}

void ijkff_set_playback_volume(IjkFFMediaPlayer *fp, float volume)
{
    assert(fp);
    ijkmp_set_playback_volume(fp->mp, volume);
}

float ijkff_get_playback_volume(IjkFFMediaPlayer *fp)
{
    assert(fp);
    return ijkmp_get_property_float(fp->mp, FFP_PROP_FLOAT_PLAYBACK_VOLUME, 1.0f);
}

void ijkff_set_stream_selected(IjkFFMediaPlayer *fp, int strean, bool selected)
{
    assert(fp);
    ijkmp_set_stream_selected(fp->mp, strean, selected);
}

float ijkff_get_float_property(IjkFFMediaPlayer *fp, int property, float dfault)
{
    assert(fp);
    return ijkmp_get_property_float(fp->mp, property, dfault);
}

int64_t ijkff_get_long_property(IjkFFMediaPlayer *fp, int property, int64_t dfault)
{
    assert(fp);
    return ijkmp_get_property_int64(fp->mp, property, dfault);
}


void ijkff_set_option(IjkFFMediaPlayer *fp, const char *value, const char *key, int category)
{
    assert(fp);
    ijkmp_set_option(fp->mp, category, key, value);
}

void ijkff_set_int_option(IjkFFMediaPlayer *fp, int64_t value, const char *key, int category)
{
    assert(fp);
    ijkmp_set_option_int(fp->mp, category, key, value);
}

void ijkff_set_window(IjkFFMediaPlayer *fp, void *window)
{
    assert(fp);
    ijkmp_set_window(fp->mp, window);
}

void ijkff_set_event_cb(IjkFFMediaPlayer *fp, void *userdata, ijkff_event_cb cb)
{
    assert(fp);
    fp->event_cb_data = userdata;
    fp->event_cb = cb;
}

int ijkplayer_overlay_draw(void *userdata,
    int w, int h, int sar_num, int sar_den,
    int planes, uint16_t *linesize, uint8_t **pixels)
{
    IjkFFMediaPlayer *fp = userdata;
    int ret = -1;
    if (fp->mp && fp->overlay_cb) {
        IjkFFOverlay *overlay = &(fp->overlay);
        overlay->w = w;
        overlay->h = h;
        overlay->sar_num = sar_num;
        overlay->sar_den = sar_den;
        overlay->planes = planes;
        overlay->pitches = linesize;
        overlay->pixels = pixels;
        fp->overlay_cb(fp->overlay_cb_data, overlay);
        ret = 0;
    }
    return ret;
}

void ijkff_set_overlay_cb(IjkFFMediaPlayer *fp, void *userdata, ijkff_overlay_cb cb)
{
    assert(fp);
    memset(&(fp->overlay), 0, sizeof(IjkFFOverlay));
    fp->overlay_cb = cb;
    fp->overlay_cb_data = userdata;
    ijkmp_set_video_callback(fp->mp, fp, ijkplayer_overlay_draw);
}

void ijkff_log_level(int level)
{
    ijkmp_global_set_log_level(level);
}

const char *ijkff_version()
{
    return ijkmp_version();
}
