/*
 * ijkplayer_android.c
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

#include "ijkplayer_android.h"

#include <assert.h>
#include "ijksdl/ijksdl_android.h"
#include "../ff_fferror.h"
#include "../ff_ffplay.h"

#define MP_RET_IF_FAILED(ret) \
    do { \
        int retval = ret; \
        if (retval != 0) return (retval); \
    } while(0)

#define MPST_RET_IF_EQ_INT(real, expected, errcode) \
    do { \
        if ((real) == (expected)) return (errcode); \
    } while(0)

#define MPST_RET_IF_EQ(real, expected) \
    MPST_RET_IF_EQ_INT(real, expected, EIJK_INVALID_STATE)

typedef struct IjkMediaPlayer {
    volatile int ref_count;
    pthread_mutex_t mutex;
    FFPlayer *ffplayer;

    void *(*msg_loop)(void*);
    pthread_t msg_thread;

    int mp_state;
    char *data_source;
    void *weak_thiz;
} IjkMediaPlayer;

inline static void ijkmp_destroy(IjkMediaPlayer *mp)
{
    if (!mp)
        return;

    ffp_destroy_p(&mp->ffplayer);

    pthread_mutex_destroy(&mp->mutex);

    av_freep(&mp->data_source);
    memset(mp, 0, sizeof(IjkMediaPlayer));
    av_freep(&mp);
}

inline static void ijkmp_destroy_p(IjkMediaPlayer **pmp)
{
    if (!pmp)
        return;

    ijkmp_destroy(*pmp);
    *pmp = NULL;
}

void ijkmp_global_init()
{
    ffp_global_init();
}

void ijkmp_global_uninit()
{
    ffp_global_uninit();
}

IjkMediaPlayer *ijkmp_create(void *(*msg_loop)(void*))
{
    IjkMediaPlayer *mp = (IjkMediaPlayer *) av_mallocz(sizeof(IjkMediaPlayer));
    if (!mp)
        goto fail;

    mp->ffplayer = ffp_create();
    if (!mp)
        goto fail;

    mp->ffplayer->vout = SDL_VoutAndroid_CreateForAndroidSurface();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->ffplayer->aout = SDL_AoutAndroid_CreateForAudioTrack();
    if (!mp->ffplayer->vout)
        goto fail;

    mp->msg_loop = msg_loop;

    ijkmp_inc_ref(mp);
    pthread_mutex_init(&mp->mutex, NULL);

    return mp;

    fail:
    ijkmp_destroy_p(&mp);
    return NULL;
}

void ijkmp_shutdown_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPTRACE("ijkmp_shutdown_l()");
    if (mp->ffplayer) {
        ffp_stop_l(mp->ffplayer);
        ffp_wait_stop_l(mp->ffplayer);
    }
    MPTRACE("ijkmp_shutdown_l()=void");
}

void ijkmp_shutdown(IjkMediaPlayer *mp)
{
    return ijkmp_shutdown_l(mp);
}

void ijkmp_reset_l(IjkMediaPlayer *mp)
{
    assert(mp);

    ijkmp_shutdown_l(mp);
    ffp_reset_internal(mp->ffplayer);

    av_freep(&mp->data_source);
    mp->mp_state = MP_STATE_IDLE;
}

void ijkmp_reset(IjkMediaPlayer *mp)
{
    assert(mp);

    MPTRACE("ijkmp_reset()");
    pthread_mutex_lock(&mp->mutex);
    ijkmp_reset_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_reset()=void");
}

void ijkmp_inc_ref(IjkMediaPlayer *mp)
{
    assert(mp);
    __sync_fetch_and_add(&mp->ref_count, 1);
}

void ijkmp_dec_ref(IjkMediaPlayer *mp)
{
    if (!mp)
        return;

    int ref_count = __sync_fetch_and_sub(&mp->ref_count, 1);
    if (ref_count == 0) {
        MPTRACE("ijkmp_dec_ref(): ref=0");
        ijkmp_shutdown(mp);
        ijkmp_destroy_p(&mp);
    }
}

void ijkmp_dec_ref_p(IjkMediaPlayer **pmp)
{
    if (!pmp)
        return;

    ijkmp_dec_ref(*pmp);
    *pmp = NULL;
}

static int ijkmp_set_data_source_l(IjkMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);

    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    char *dup_url = strdup(url);
    if (!dup_url)
        return EIJK_OUT_OF_MEMORY;

    av_freep(&mp->data_source);
    mp->data_source = av_strdup(url);
    mp->mp_state = MP_STATE_INITIALIZED;
    return 0;
}

int ijkmp_set_data_source(IjkMediaPlayer *mp, const char *url)
{
    assert(mp);
    assert(url);
    MPTRACE("ijkmp_set_data_source(url=\"%s\")", url);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_set_data_source_l(mp, url);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_set_data_source(url=\"%s\")=%d", url, retval);
    return retval;
}

static int ijkmp_prepare_async_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    assert(mp->data_source);

    mp->mp_state = MP_STATE_ASYNC_PREPARING;

    msg_queue_start(&mp->ffplayer->msg_queue);
    ijkmp_inc_ref(mp);
    pthread_create(&mp->msg_thread, NULL, mp->msg_loop, mp);
    // TODO: 9 release weak_thiz if pthread_create() failed;

    int retval = ffp_prepare_async_l(mp->ffplayer, mp->data_source);
    if (retval < 0) {
        mp->mp_state = MP_STATE_ERROR;
        return retval;
    }

    return 0;
}

int ijkmp_prepare_async(IjkMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("ijkmp_prepare_async()");
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_prepare_async_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_prepare_async()=%d", retval);
    return retval;
}

static int ikjmp_chkst_start_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

static int ijkmp_start_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_start_l(mp->mp_state));

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    ffp_notify_msg(mp->ffplayer, FFP_REQ_START, 0, 0);

    return 0;
}

int ijkmp_start(IjkMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("ijkmp_start()");
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_start_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_start()=%d", retval);
    return retval;
}

static int ikjmp_chkst_pause_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

static int ijkmp_pause_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_pause_l(mp->mp_state));

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    ffp_notify_msg(mp->ffplayer, FFP_REQ_PAUSE, 0, 0);

    return 0;
}

int ijkmp_pause(IjkMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("ijkmp_pause()");
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_pause_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_pause()=%d", retval);
    return retval;
}

static int ijkmp_stop_l(IjkMediaPlayer *mp)
{
    assert(mp);

    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_INITIALIZED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_COMPLETED);
    // MPST_RET_IF_EQ(mp->mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp->mp_state, MP_STATE_END);

    ffp_remove_msg(mp->ffplayer, FFP_REQ_START);
    ffp_remove_msg(mp->ffplayer, FFP_REQ_PAUSE);
    int retval = ffp_stop_l(mp->ffplayer);
    if (retval < 0) {
        return retval;
    }

    // FIXME: 9 change to MP_STATE_STOPPED in read_thread() */
    mp->mp_state = MP_STATE_STOPPED;
    return 0;
}

int ijkmp_stop(IjkMediaPlayer *mp)
{
    assert(mp);
    MPTRACE("ijkmp_stop()");
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_stop_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_stop()=%d", retval);
    return retval;
}

bool ijkmp_is_playing(IjkMediaPlayer *mp)
{
    assert(mp);
    if (mp->mp_state == MP_STATE_PREPARED ||
        mp->mp_state == MP_STATE_STARTED) {
        return true;
    }

    return false;
}

static int ikjmp_chkst_seek_l(int mp_state)
{
    MPST_RET_IF_EQ(mp_state, MP_STATE_IDLE);
    MPST_RET_IF_EQ(mp_state, MP_STATE_INITIALIZED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_ASYNC_PREPARING);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PREPARED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_STARTED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_PAUSED);
    // MPST_RET_IF_EQ(mp_state, MP_STATE_COMPLETED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_STOPPED);
    MPST_RET_IF_EQ(mp_state, MP_STATE_ERROR);
    MPST_RET_IF_EQ(mp_state, MP_STATE_END);

    return 0;
}

int ijkmp_seek_to_l(IjkMediaPlayer *mp, long msec)
{
    assert(mp);

    MP_RET_IF_FAILED(ikjmp_chkst_seek_l(mp->mp_state));

    ffp_remove_msg(mp->ffplayer, FFP_REQ_SEEK);
    ffp_notify_msg(mp->ffplayer, FFP_REQ_SEEK, msec, 0);
    // TODO: 9 64-bit long?

    return 0;
}

int ijkmp_seek_to(IjkMediaPlayer *mp, long msec)
{
    assert(mp);
    MPTRACE("ijkmp_seek_to(%ld)", msec);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_seek_to_l(mp, msec);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_seek_to(%ld)=%d", msec, retval);

    return retval;
}

static long ijkmp_get_current_position_l(IjkMediaPlayer *mp)
{
    return ffp_get_current_position_l(mp->ffplayer);
}

long ijkmp_get_current_position(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    long retval = ijkmp_get_current_position_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

static long ijkmp_get_duration_l(IjkMediaPlayer *mp)
{
    return ffp_get_duration_l(mp->ffplayer);
}

long ijkmp_get_duration(IjkMediaPlayer *mp)
{
    assert(mp);
    pthread_mutex_lock(&mp->mutex);
    int retval = ijkmp_get_duration_l(mp);
    pthread_mutex_unlock(&mp->mutex);
    return retval;
}

void ijkmp_set_android_surface_l(JNIEnv *env, IjkMediaPlayer *mp, jobject android_surface)
{
    assert(mp);
    assert(mp->ffplayer);
    assert(mp->ffplayer->vout);

    SDL_VoutAndroid_SetAndroidSurface(env, mp->ffplayer->vout, android_surface);
}

void ijkmp_set_android_surface(JNIEnv *env, IjkMediaPlayer *mp, jobject android_surface)
{
    assert(mp);
    MPTRACE("ijkmp_set_android_surface(surface=%p)", (void*)android_surface);
    pthread_mutex_lock(&mp->mutex);
    ijkmp_set_android_surface_l(env, mp, android_surface);
    pthread_mutex_unlock(&mp->mutex);
    MPTRACE("ijkmp_set_android_surface(surface=%p)=void", (void*)android_surface);
}

void *ijkmp_set_weak_thiz(IjkMediaPlayer *mp, void *weak_thiz)
{
    void *prev_weak_thiz = mp->weak_thiz;

    mp->weak_thiz = weak_thiz;

    return prev_weak_thiz;
}

int ijkmp_get_msg(IjkMediaPlayer *mp, AVMessage *msg, int block)
{
    assert(mp);
    while (1) {
        int continue_wait_next_msg = 0;
        int retval = msg_queue_get(&mp->ffplayer->msg_queue, msg, block);
        if (retval <= 0)
            return retval;

        switch (msg->what) {
        case FFP_MSG_PREPARED:
            MPTRACE("ijkmp_get_msg: FFP_MSG_PREPARED");
            pthread_mutex_lock(&mp->mutex);
            if (mp->mp_state == MP_STATE_ASYNC_PREPARING) {
                mp->mp_state = MP_STATE_PREPARED;
            } else {
                // FIXME: 1: onError() ?
                ALOGE("FFP_MSG_PREPARED: expecting mp_state==MP_STATE_ASYNC_PREPARING");
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_MSG_COMPLETED:
            MPTRACE("ijkmp_get_msg: FFP_MSG_COMPLETED");
            pthread_mutex_lock(&mp->mutex);
            ffp_pause_l(mp->ffplayer);
            mp->mp_state = MP_STATE_COMPLETED;
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_START:
            MPTRACE("ijkmp_get_msg: FFP_REQ_START");
            continue_wait_next_msg = 1;
            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_start_l(mp->mp_state)) {
                // FIXME: 8 check seekable
                if (mp->mp_state == MP_STATE_COMPLETED)
                    ffp_seek_to_l(mp->ffplayer, 0);

                int retval = ffp_start_l(mp->ffplayer);
                if (retval == 0) {
                    mp->mp_state = MP_STATE_STARTED;
                }
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_PAUSE:
            MPTRACE("ijkmp_get_msg: FFP_REQ_PAUSE");
            continue_wait_next_msg = 1;
            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_pause_l(mp->mp_state)) {
                int retval = ffp_pause_l(mp->ffplayer);
                if (retval == 0)
                    mp->mp_state = MP_STATE_PAUSED;
            }
            pthread_mutex_unlock(&mp->mutex);
            break;

        case FFP_REQ_SEEK:
            MPTRACE("ijkmp_get_msg: FFP_REQ_SEEK");
            continue_wait_next_msg = 1;
            pthread_mutex_lock(&mp->mutex);
            if (0 == ikjmp_chkst_seek_l(mp->mp_state)) {
                ffp_seek_to_l(mp->ffplayer, msg->arg1);
            }
            pthread_mutex_unlock(&mp->mutex);
            break;
        }

        if (continue_wait_next_msg)
            continue;

        return retval;
    }

    return -1;
}
