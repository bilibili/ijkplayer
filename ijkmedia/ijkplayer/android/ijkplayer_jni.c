/*
 * ijkplayer_jni.c
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

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <jni.h>
#include "ijkutil/ijkutil.h"
#include "ijkutil/android/ijkutil_android.h"
#include "../ff_ffplay.h"
#include "ffmpeg_api_jni.h"
#include "ijkplayer_android_def.h"
#include "ijkplayer_android.h"
#include "ijksdl/android/android_arraylist.h"
#include "ijksdl/android/android_bundle.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijksdl/android/ijksdl_codec_android_mediadef.h"

#define JNI_MODULE_PACKAGE      "tv/danmaku/ijk/media/player"
#define JNI_CLASS_IJKPLAYER     "tv/danmaku/ijk/media/player/IjkMediaPlayer"
#define JNI_IJK_MEDIA_EXCEPTION "tv/danmaku/ijk/media/player/IjkMediaException"

#define IJK_CHECK_MPRET_GOTO(retval, env, label) \
    JNI_CHECK_GOTO((retval != EIJK_INVALID_STATE), env, "java/lang/IllegalStateException", NULL, label); \
    JNI_CHECK_GOTO((retval != EIJK_OUT_OF_MEMORY), env, "java/lang/OutOfMemoryError", NULL, label); \
    JNI_CHECK_GOTO((retval == 0), env, JNI_IJK_MEDIA_EXCEPTION, NULL, label);

static JavaVM* g_jvm;

typedef struct player_fields_t {
    pthread_mutex_t mutex;
    jclass clazz;

    jfieldID mNativeMediaPlayer;

    jfieldID surface_texture;

    jmethodID jmid_postEventFromNative;
    jmethodID jmid_onSelectCodec;
    jmethodID jmid_onControlResolveSegmentCount;
    jmethodID jmid_onControlResolveSegmentUrl;
    jmethodID jmid_onControlResolveSegmentOfflineMrl;
    jmethodID jmid_onControlResolveSegmentDuration;
} player_fields_t;
static player_fields_t g_clazz;

static int format_control_message(void *opaque, int type, void *data, size_t data_size);
static bool mediacodec_select_callback(void *opaque, ijkmp_mediacodecinfo_context *mcc);

static IjkMediaPlayer *jni_get_media_player(JNIEnv* env, jobject thiz)
{
    pthread_mutex_lock(&g_clazz.mutex);

    IjkMediaPlayer *mp = (IjkMediaPlayer *) (intptr_t) (*env)->GetLongField(env, thiz, g_clazz.mNativeMediaPlayer);
    if (mp) {
        ijkmp_inc_ref(mp);
    }

    pthread_mutex_unlock(&g_clazz.mutex);
    return mp;
}

static IjkMediaPlayer *jni_set_media_player(JNIEnv* env, jobject thiz, IjkMediaPlayer *mp)
{
    pthread_mutex_lock(&g_clazz.mutex);

    IjkMediaPlayer *old = (IjkMediaPlayer*) (intptr_t) (*env)->GetLongField(env, thiz, g_clazz.mNativeMediaPlayer);
    if (mp) {
        ijkmp_inc_ref(mp);
    }
    (*env)->SetLongField(env, thiz, g_clazz.mNativeMediaPlayer, (intptr_t) mp);

    pthread_mutex_unlock(&g_clazz.mutex);

    // NOTE: ijkmp_dec_ref may block thread
    if (old != NULL ) {
        ijkmp_dec_ref_p(&old);
    }

    return old;
}

static int message_loop(void *arg);

static void
IjkMediaPlayer_setDataSourceAndHeaders(
    JNIEnv *env, jobject thiz, jstring path,
    jobjectArray keys, jobjectArray values)
{
    MPTRACE("IjkMediaPlayer_setDataSourceAndHeaders");
    int retval = 0;
    const char *c_path = NULL;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(path, env, "java/lang/IllegalArgumentException", "mpjni: setDataSource: null path", LABEL_RETURN);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setDataSource: null mp", LABEL_RETURN);

    c_path = (*env)->GetStringUTFChars(env, path, NULL );
    JNI_CHECK_GOTO(c_path, env, "java/lang/OutOfMemoryError", "mpjni: setDataSource: path.string oom", LABEL_RETURN);

    ALOGV("setDataSource: path %s", c_path);
    retval = ijkmp_set_data_source(mp, c_path);
    (*env)->ReleaseStringUTFChars(env, path, c_path);

    IJK_CHECK_MPRET_GOTO(retval, env, LABEL_RETURN);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    MPTRACE("IjkMediaPlayer_setVideoSurface");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: setVideoSurface: null mp", LABEL_RETURN);

    ijkmp_android_set_surface(env, mp, jsurface);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return;
}

static void
IjkMediaPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{
    MPTRACE("IjkMediaPlayer_prepareAsync");
    int retval = 0;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: prepareAsync: null mp", LABEL_RETURN);

    retval = ijkmp_prepare_async(mp);
    IJK_CHECK_MPRET_GOTO(retval, env, LABEL_RETURN);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_start(JNIEnv *env, jobject thiz)
{
    MPTRACE("IjkMediaPlayer_start");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: start: null mp", LABEL_RETURN);

    ijkmp_start(mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_stop(JNIEnv *env, jobject thiz)
{
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: stop: null mp", LABEL_RETURN);

    ijkmp_stop(mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_pause(JNIEnv *env, jobject thiz)
{
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: pause: null mp", LABEL_RETURN);

    ijkmp_pause(mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_seekTo(JNIEnv *env, jobject thiz, int msec)
{
    MPTRACE("IjkMediaPlayer_seekTo");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: seekTo: null mp", LABEL_RETURN);

    ijkmp_seek_to(mp, msec);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static jboolean
IjkMediaPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    jboolean retval = JNI_FALSE;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: isPlaying: null mp", LABEL_RETURN);

    retval = ijkmp_is_playing(mp) ? JNI_TRUE : JNI_FALSE;

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}

static int
IjkMediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    int retval = 0;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: getCurrentPosition: null mp", LABEL_RETURN);

    retval = ijkmp_get_current_position(mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}

static int
IjkMediaPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    int retval = 0;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: getDuration: null mp", LABEL_RETURN);

    retval = ijkmp_get_duration(mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    return retval;
}

static void
IjkMediaPlayer_release(JNIEnv *env, jobject thiz)
{
    MPTRACE("IjkMediaPlayer_release");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    if (!mp)
        return;

    // explicit shutdown mp, in case it is not the last mp-ref here
    ijkmp_android_set_surface(env, mp, NULL );
    ijkmp_shutdown(mp);
    //only delete weak_thiz at release
    jobject weak_thiz = (jobject) ijkmp_set_weak_thiz(mp, NULL );
    (*env)->DeleteGlobalRef(env, weak_thiz);
    jni_set_media_player(env, thiz, NULL );

    ijkmp_dec_ref_p(&mp);
}

static void IjkMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this);
static void
IjkMediaPlayer_reset(JNIEnv *env, jobject thiz)
{
    MPTRACE("IjkMediaPlayer_reset");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    if (!mp)
        return;

    jobject weak_thiz = (jobject) ijkmp_set_weak_thiz(mp, NULL );

    IjkMediaPlayer_release(env, thiz);
    IjkMediaPlayer_native_setup(env, thiz, weak_thiz);

    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setVolume(JNIEnv *env, jobject thiz, jfloat leftVolume, jfloat rightVolume)
{
    MPTRACE("IjkMediaPlayer_setVolume");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: setVolume: null mp", LABEL_RETURN);

    ijkmp_android_set_volume(env, mp, leftVolume, rightVolume);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setAvFormatOption(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("IjkMediaPlayer_setAvFormatOption");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    const char *c_name = NULL;
    const char *c_value = NULL;
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvFormatOption: null mp", LABEL_RETURN);

    c_name = (*env)->GetStringUTFChars(env, name, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvFormatOption: name.string oom", LABEL_RETURN);

    c_value = (*env)->GetStringUTFChars(env, value, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvFormatOption: name.string oom", LABEL_RETURN);

    ijkmp_set_format_option(mp, c_name, c_value);

    LABEL_RETURN:
    if (c_name)
        (*env)->ReleaseStringUTFChars(env, name, c_name);
    if (c_value)
        (*env)->ReleaseStringUTFChars(env, value, c_value);
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setAvCodecOption(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("IjkMediaPlayer_setAvCodecOption");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    const char *c_name = NULL;
    const char *c_value = NULL;
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvCodecOption: null mp", LABEL_RETURN);

    c_name = (*env)->GetStringUTFChars(env, name, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvCodecOption: name.string oom", LABEL_RETURN);

    c_value = (*env)->GetStringUTFChars(env, value, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setAvCodecOption: name.string oom", LABEL_RETURN);

    ijkmp_set_codec_option(mp, c_name, c_value);

    LABEL_RETURN:
    if (c_name)
        (*env)->ReleaseStringUTFChars(env, name, c_name);
    if (c_value)
        (*env)->ReleaseStringUTFChars(env, value, c_value);
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setSwScaleOption(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("IjkMediaPlayer_setSwScaleOption");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    const char *c_name = NULL;
    const char *c_value = NULL;
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setSwScaleOption: null mp", LABEL_RETURN);

    c_name = (*env)->GetStringUTFChars(env, name, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setSwScaleOption: name.string oom", LABEL_RETURN);

    c_value = (*env)->GetStringUTFChars(env, value, NULL );
    JNI_CHECK_GOTO(c_name, env, "java/lang/OutOfMemoryError", "mpjni: setSwScaleOption: name.string oom", LABEL_RETURN);

    ijkmp_set_sws_option(mp, c_name, c_value);

    LABEL_RETURN:
    if (c_name)
        (*env)->ReleaseStringUTFChars(env, name, c_name);
    if (c_value)
        (*env)->ReleaseStringUTFChars(env, value, c_value);
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setOverlayFormat(JNIEnv *env, jobject thiz, jint chromaFourCC)
{
    MPTRACE("IjkMediaPlayer_setOverlayFormat");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAvCodecOption: null mp", LABEL_RETURN);

    ijkmp_set_overlay_format(mp, chromaFourCC);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setFrameDrop(JNIEnv *env, jobject thiz, jint frameDrop)
{
    MPTRACE("IjkMediaPlayer_setFrameDrop");
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setFrameDrop: null mp", LABEL_RETURN);

    ijkmp_set_framedrop(mp, frameDrop);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setMediaCodecEnabled(JNIEnv *env, jobject thiz, jboolean enabled)
{
    MPTRACE("%s", __func__);
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setMediaCodecEnabled: null mp", LABEL_RETURN);

    ijkmp_android_set_mediacodec_enabled(mp, enabled);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setOpenSLESEnabled(JNIEnv *env, jobject thiz, jboolean enabled)
{
    MPTRACE("%s", __func__);
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setOpenSLESEnabled: null mp", LABEL_RETURN);

    ijkmp_android_set_opensles_enabled(mp, enabled);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_setAutoPlayOnPrepared(JNIEnv *env, jobject thiz, jboolean enabled)
{
    MPTRACE("%s", __func__);
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: setAutoPlayOnPrepared: null mp", LABEL_RETURN);

    ijkmp_set_auto_play_on_prepared(mp, enabled);

LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static jstring
IjkMediaPlayer_getColorFormatName(JNIEnv *env, jclass clazz, jint mediaCodecColorFormat)
{
    const char *codec_name = SDL_AMediaCodec_getColorFormatName(mediaCodecColorFormat);
    if (!codec_name)
        return NULL ;

    return (*env)->NewStringUTF(env, codec_name);
}

static jstring
IjkMediaPlayer_getVideoCodecInfo(JNIEnv *env, jobject thiz)
{
    MPTRACE("%s", __func__);
    jstring jcodec_info = NULL;
    int ret = 0;
    char *codec_info = NULL;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: getVideoCodecInfo: null mp", LABEL_RETURN);

    ret = ijkmp_get_video_codec_info(mp, &codec_info);
    if (ret < 0 || !codec_info)
        goto LABEL_RETURN;

    jcodec_info = (*env)->NewStringUTF(env, codec_info);
    LABEL_RETURN:
    if (codec_info)
        free(codec_info);

    ijkmp_dec_ref_p(&mp);
    return jcodec_info;
}

static jstring
IjkMediaPlayer_getAudioCodecInfo(JNIEnv *env, jobject thiz)
{
    MPTRACE("%s", __func__);
    jstring jcodec_info = NULL;
    int ret = 0;
    char *codec_info = NULL;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: getAudioCodecInfo: null mp", LABEL_RETURN);

    ret = ijkmp_get_audio_codec_info(mp, &codec_info);
    if (ret < 0 || !codec_info)
        goto LABEL_RETURN;

    jcodec_info = (*env)->NewStringUTF(env, codec_info);
    LABEL_RETURN:
    if (codec_info)
        free(codec_info);

    ijkmp_dec_ref_p(&mp);
    return jcodec_info;
}

inline static void fillMetaInternal(JNIEnv *env, jobject jbundle, IjkMediaMeta *meta, const char *key, const char *default_value)
{
    const char *value = ijkmeta_get_string_l(meta, key);
    if (value == NULL )
        value = default_value;

    ASDK_Bundle__putString_c(env, jbundle, key, value);
    SDL_JNI_CatchException(env);
}

static jobject
IjkMediaPlayer_getMediaMeta(JNIEnv *env, jobject thiz)
{
    MPTRACE("%s", __func__);
    bool is_locked = false;
    jobject jret_bundle = NULL;
    jobject jlocal_bundle = NULL;
    jobject jstream_bundle = NULL;
    jobject jarray_list = NULL;
    IjkMediaMeta *meta = NULL;
    IjkMediaPlayer *mp = jni_get_media_player(env, thiz);
    JNI_CHECK_GOTO(mp, env, "java/lang/IllegalStateException", "mpjni: getMediaMeta: null mp", LABEL_RETURN);

    meta = ijkmp_get_meta_l(mp);
    if (!meta)
        goto LABEL_RETURN;

    ijkmeta_lock(meta);
    is_locked = true;

    jlocal_bundle = ASDK_Bundle__init(env);
    if (SDL_JNI_RethrowException(env)) {
        goto LABEL_RETURN;
    }

    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_FORMAT, NULL );
    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_DURATION_US, NULL );
    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_START_US, NULL );
    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_START_US, NULL );

    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_VIDEO_STREAM, "-1");
    fillMetaInternal(env, jlocal_bundle, meta, IJKM_KEY_AUDIO_STREAM, "-1");

    jarray_list = ASDK_ArrayList__init(env);
    if (SDL_JNI_RethrowException(env)) {
        goto LABEL_RETURN;
    }

    size_t count = ijkmeta_get_children_count_l(meta);
    for (size_t i = 0; i < count; ++i) {
        IjkMediaMeta *streamRawMeta = ijkmeta_get_child_l(meta, i);
        if (streamRawMeta) {
            jstream_bundle = ASDK_Bundle__init(env);
            if (SDL_JNI_RethrowException(env)) {
                goto LABEL_RETURN;
            }

            fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_TYPE, IJKM_VAL_TYPE__UNKNOWN);
            const char *type = ijkmeta_get_string_l(streamRawMeta, IJKM_KEY_TYPE);
            if (type) {
                fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_CODEC_NAME, NULL );
                fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_CODEC_PROFILE, NULL );
                fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_CODEC_LONG_NAME, NULL );
                fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_BITRATE, NULL );

                if (0 == strcmp(type, IJKM_VAL_TYPE__VIDEO)) {
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_WIDTH, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_HEIGHT, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_FPS_NUM, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_FPS_DEN, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_TBR_NUM, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_TBR_DEN, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_SAR_NUM, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_SAR_DEN, NULL );
                } else if (0 == strcmp(type, IJKM_VAL_TYPE__AUDIO)) {
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_SAMPLE_RATE, NULL );
                    fillMetaInternal(env, jstream_bundle, streamRawMeta, IJKM_KEY_CHANNEL_LAYOUT, NULL );
                }
                ASDK_ArrayList__add(env, jarray_list, jstream_bundle);
                if (SDL_JNI_RethrowException(env)) {
                    goto LABEL_RETURN;
                }
            }

            SDL_JNI_DeleteLocalRefP(env, &jstream_bundle);
        }
    }

    ASDK_Bundle__putParcelableArrayList_c(env, jlocal_bundle, IJKM_KEY_STREAMS, jarray_list);
    jret_bundle = jlocal_bundle;
    jlocal_bundle = NULL;
    LABEL_RETURN:
    if (is_locked && meta)
        ijkmeta_unlock(meta);

    SDL_JNI_DeleteLocalRefP(env, &jstream_bundle);
    SDL_JNI_DeleteLocalRefP(env, &jlocal_bundle);
    SDL_JNI_DeleteLocalRefP(env, &jarray_list);

    ijkmp_dec_ref_p(&mp);
    return jret_bundle;
}

static void
IjkMediaPlayer_native_init(JNIEnv *env)
{
    MPTRACE("IjkMediaPlayer_native_init");
}

static void
IjkMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    MPTRACE("IjkMediaPlayer_native_setup");
    IjkMediaPlayer *mp = ijkmp_android_create(message_loop);
    JNI_CHECK_GOTO(mp, env, "java/lang/OutOfMemoryError", "mpjni: native_setup: ijkmp_create() failed", LABEL_RETURN);

    jni_set_media_player(env, thiz, mp);
    ijkmp_set_weak_thiz(mp, (*env)->NewGlobalRef(env, weak_this));
    ijkmp_set_format_callback(mp, format_control_message, (*env)->NewGlobalRef(env, weak_this));
    ijkmp_android_set_mediacodec_select_callback(mp, mediacodec_select_callback, (*env)->NewGlobalRef(env, weak_this));

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
}

static void
IjkMediaPlayer_native_finalize(JNIEnv *env, jobject thiz, jobject name, jobject value)
{
    MPTRACE("IjkMediaPlayer_native_finalize");
    IjkMediaPlayer_release(env, thiz);
}

static int
_onNativeControlResolveSegmentConcat(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL )
        return -1;

    IJKFormatSegmentConcatContext *fsc_concat = (IJKFormatSegmentConcatContext *) data;

    jint count = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.jmid_onControlResolveSegmentCount, weak_thiz);
    if (count <= 0)
        return -1;

    fsc_concat->count = count;
    return 0;
}

static int
_onNativeControlResolveSegment(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL )
        return -1;

    IJKFormatSegmentContext *fsc = (IJKFormatSegmentContext *) data;

    jint duration = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.jmid_onControlResolveSegmentDuration, weak_thiz, fsc->position);

    jstring url = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_onControlResolveSegmentUrl, weak_thiz, fsc->position);
    if (url == NULL )
        return -1;

    const char* c_url = (*env)->GetStringUTFChars(env, url, NULL );
    if (c_url == NULL )
        return -1;

    fsc->url = strdup(c_url);
    (*env)->ReleaseStringUTFChars(env, url, c_url);

    if (fsc->url == NULL )
        return -1;

    fsc->duration = duration;
    fsc->duration *= 1000;
    fsc->url_free = free;
    return 0;
}

static int
_onNativeControlResolveSegmentOffline(JNIEnv *env, jobject weak_thiz, int type, void *data, size_t data_size)
{
    if (weak_thiz == NULL || data == NULL )
        return -1;

    IJKFormatSegmentContext *fsc = (IJKFormatSegmentContext *) data;
    jint duration = (*env)->CallStaticIntMethod(env, g_clazz.clazz, g_clazz.jmid_onControlResolveSegmentDuration, weak_thiz, fsc->position);

    jstring mrl = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_onControlResolveSegmentOfflineMrl, weak_thiz, fsc->position);
    if (mrl == NULL )
        return -1;

    const char* c_mrl = (*env)->GetStringUTFChars(env, mrl, NULL );
    if (c_mrl == NULL )
        return -1;

    fsc->url = strdup(c_mrl);
    (*env)->ReleaseStringUTFChars(env, mrl, c_mrl);

    if (fsc->url == NULL )
        return -1;

    fsc->duration = duration;
    fsc->duration *= 1000;
    fsc->url_free = free;
    return 0;
}

// NOTE: support to be called from read_thread
static int
format_control_message(void *opaque, int type, void *data, size_t data_size)
{
    JNIEnv *env = NULL;
    SDL_JNI_SetupThreadEnv(&env);

    jobject weak_thiz = (jobject) opaque;
    if (weak_thiz == NULL )
        return -1;

    switch (type) {
    case IJKAVF_CM_RESOLVE_SEGMENT_CONCAT:
        return _onNativeControlResolveSegmentConcat(env, weak_thiz, type, data, data_size);
    case IJKAVF_CM_RESOLVE_SEGMENT:
        return _onNativeControlResolveSegment(env, weak_thiz, type, data, data_size);
    case IJKAVF_CM_RESOLVE_SEGMENT_OFFLINE:
        return _onNativeControlResolveSegmentOffline(env, weak_thiz, type, data, data_size);
    default: {
        return -1;
    }
    }

    return -1;
}

static bool mediacodec_select_callback(void *opaque, ijkmp_mediacodecinfo_context *mcc)
{
    JNIEnv *env = NULL;
    jobject jmime = NULL;
    jstring jcodec_name = NULL;
    jobject weak_this = (jobject) opaque;
    const char *codec_name = NULL;
    bool found_codec = false;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("%s: SetupThreadEnv failed\n", __func__);
        return -1;
    }

    jmime = (*env)->NewStringUTF(env, mcc->mime_type);
    if (SDL_JNI_CatchException(env) || !jmime) {
        goto fail;
    }

    jcodec_name = (*env)->CallStaticObjectMethod(env, g_clazz.clazz, g_clazz.jmid_onSelectCodec, weak_this, jmime, mcc->profile, mcc->level);
    if (SDL_JNI_CatchException(env) || !jcodec_name) {
        goto fail;
    }

    codec_name = (*env)->GetStringUTFChars(env, jcodec_name, NULL );
    if (!codec_name || !*codec_name) {
        goto fail;
    }

    strncpy(mcc->codec_name, codec_name, sizeof(mcc->codec_name) / sizeof(*mcc->codec_name));
    mcc->codec_name[sizeof(mcc->codec_name) / sizeof(*mcc->codec_name) - 1] = 0;
    found_codec = true;
    fail:
    if (codec_name) {
        (*env)->ReleaseStringUTFChars(env, jcodec_name, codec_name);
        codec_name = NULL;
    }

    SDL_JNI_DeleteLocalRefP(env, &jcodec_name);
    SDL_JNI_DeleteLocalRefP(env, &jmime);
    return found_codec;
}

inline static void post_event(JNIEnv *env, jobject weak_this, int what, int arg1, int arg2)
{
    // MPTRACE("post_event(%p, %p, %d, %d, %d)", (void*)env, (void*) weak_this, what, arg1, arg2);
    (*env)->CallStaticVoidMethod(env, g_clazz.clazz, g_clazz.jmid_postEventFromNative, weak_this, what, arg1, arg2, NULL );
    // MPTRACE("post_event()=void");
}

static void message_loop_n(JNIEnv *env, IjkMediaPlayer *mp)
{
    jobject weak_thiz = (jobject) ijkmp_get_weak_thiz(mp);
    JNI_CHECK_GOTO(weak_thiz, env, NULL, "mpjni: message_loop_n: null weak_thiz", LABEL_RETURN);

    while (1) {
        AVMessage msg;

        int retval = ijkmp_get_msg(mp, &msg, 1);
        if (retval < 0)
            break;

        // block-get should never return 0
        assert(retval > 0);

        switch (msg.what) {
        case FFP_MSG_FLUSH:
            MPTRACE("FFP_MSG_FLUSH:");
            post_event(env, weak_thiz, MEDIA_NOP, 0, 0);
            break;
        case FFP_MSG_ERROR:
            MPTRACE("FFP_MSG_ERROR: %d", msg.arg1);
            post_event(env, weak_thiz, MEDIA_ERROR, MEDIA_ERROR_IJK_PLAYER, msg.arg1);
            break;
        case FFP_MSG_PREPARED:
            MPTRACE("FFP_MSG_PREPARED:");
            post_event(env, weak_thiz, MEDIA_PREPARED, 0, 0);
            break;
        case FFP_MSG_COMPLETED:
            MPTRACE("FFP_MSG_COMPLETED:");
            post_event(env, weak_thiz, MEDIA_PLAYBACK_COMPLETE, 0, 0);
            break;
        case FFP_MSG_VIDEO_SIZE_CHANGED:
            MPTRACE("FFP_MSG_VIDEO_SIZE_CHANGED: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_SET_VIDEO_SIZE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_SAR_CHANGED:
            MPTRACE("FFP_MSG_SAR_CHANGED: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_SET_VIDEO_SAR, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_BUFFERING_START:
            MPTRACE("FFP_MSG_BUFFERING_START:");
            post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_START, 0);
            break;
        case FFP_MSG_BUFFERING_END:
            MPTRACE("FFP_MSG_BUFFERING_END:");
            post_event(env, weak_thiz, MEDIA_INFO, MEDIA_INFO_BUFFERING_END, 0);
            break;
        case FFP_MSG_BUFFERING_UPDATE:
            // MPTRACE("FFP_MSG_BUFFERING_UPDATE: %d, %d", msg.arg1, msg.arg2);
            post_event(env, weak_thiz, MEDIA_BUFFERING_UPDATE, msg.arg1, msg.arg2);
            break;
        case FFP_MSG_BUFFERING_BYTES_UPDATE:
            break;
        case FFP_MSG_BUFFERING_TIME_UPDATE:
            break;
        case FFP_MSG_SEEK_COMPLETE:
            MPTRACE("FFP_MSG_SEEK_COMPLETE:");
            post_event(env, weak_thiz, MEDIA_SEEK_COMPLETE, 0, 0);
            break;
        case FFP_MSG_PLAYBACK_STATE_CHANGED:
            break;
        default:
            ALOGE("unknown FFP_MSG_xxx(%d)", msg.what);
            break;
        }
    }

    LABEL_RETURN:
    ;
}

static int message_loop(void *arg)
{
    MPTRACE("message_loop");

    JNIEnv *env = NULL;
    (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL );

    IjkMediaPlayer *mp = (IjkMediaPlayer*) arg;
    JNI_CHECK_GOTO(mp, env, NULL, "mpjni: native_message_loop: null mp", LABEL_RETURN);

    message_loop_n(env, mp);

    LABEL_RETURN:
    ijkmp_dec_ref_p(&mp);
    (*g_jvm)->DetachCurrentThread(g_jvm);

    MPTRACE("message_loop exit");
    return 0;
}

// ----------------------------------------------------------------------------

static JNINativeMethod g_methods[] = {
    {
        "_setDataSource",
        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
        (void *) IjkMediaPlayer_setDataSourceAndHeaders
    },
    { "_setVideoSurface", "(Landroid/view/Surface;)V", (void *) IjkMediaPlayer_setVideoSurface },
    { "_prepareAsync", "()V", (void *) IjkMediaPlayer_prepareAsync },
    { "_start", "()V", (void *) IjkMediaPlayer_start },
    { "_stop", "()V", (void *) IjkMediaPlayer_stop },
    { "seekTo", "(J)V", (void *) IjkMediaPlayer_seekTo },
    { "_pause", "()V", (void *) IjkMediaPlayer_pause },
    { "isPlaying", "()Z", (void *) IjkMediaPlayer_isPlaying },
    { "getCurrentPosition", "()J", (void *) IjkMediaPlayer_getCurrentPosition },
    { "getDuration", "()J", (void *) IjkMediaPlayer_getDuration },
    { "_release", "()V", (void *) IjkMediaPlayer_release },
    { "_reset", "()V", (void *) IjkMediaPlayer_reset },
    { "setVolume", "(FF)V", (void *) IjkMediaPlayer_setVolume },
    { "native_init", "()V", (void *) IjkMediaPlayer_native_init },
    { "native_setup", "(Ljava/lang/Object;)V", (void *) IjkMediaPlayer_native_setup },
    { "native_finalize", "()V", (void *) IjkMediaPlayer_native_finalize },

    { "_setAvFormatOption", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) IjkMediaPlayer_setAvFormatOption },
    { "_setAvCodecOption", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) IjkMediaPlayer_setAvCodecOption },
    { "_setSwScaleOption", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) IjkMediaPlayer_setSwScaleOption },
    { "_setOverlayFormat", "(I)V", (void *) IjkMediaPlayer_setOverlayFormat },
    { "_setFrameDrop", "(I)V", (void *) IjkMediaPlayer_setFrameDrop },
    { "_setMediaCodecEnabled", "(Z)V", (void *) IjkMediaPlayer_setMediaCodecEnabled },
    { "_setOpenSLESEnabled", "(Z)V", (void *) IjkMediaPlayer_setOpenSLESEnabled },
    { "_setAutoPlayOnPrepared", "(Z)V", (void *) IjkMediaPlayer_setAutoPlayOnPrepared },

    { "_getColorFormatName", "(I)Ljava/lang/String;", (void *) IjkMediaPlayer_getColorFormatName },
    { "_getVideoCodecInfo", "()Ljava/lang/String;", (void *) IjkMediaPlayer_getVideoCodecInfo },
    { "_getAudioCodecInfo", "()Ljava/lang/String;", (void *) IjkMediaPlayer_getAudioCodecInfo },
    { "_getMediaMeta", "()Landroid/os/Bundle;", (void *) IjkMediaPlayer_getMediaMeta },
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;

    g_jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    pthread_mutex_init(&g_clazz.mutex, NULL );

    // FindClass returns LocalReference
    IJK_FIND_JAVA_CLASS(env, g_clazz.clazz, JNI_CLASS_IJKPLAYER);
    (*env)->RegisterNatives(env, g_clazz.clazz, g_methods, NELEM(g_methods) );

    g_clazz.mNativeMediaPlayer = (*env)->GetFieldID(env, g_clazz.clazz, "mNativeMediaPlayer", "J");
    IJK_CHECK_RET(g_clazz.mNativeMediaPlayer, -1, "missing mNativeMediaPlayer");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_postEventFromNative, g_clazz.clazz,
        "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_onSelectCodec, g_clazz.clazz,
        "onSelectCodec", "(Ljava/lang/Object;Ljava/lang/String;II)Ljava/lang/String;");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_onControlResolveSegmentCount, g_clazz.clazz,
        "onControlResolveSegmentCount", "(Ljava/lang/Object;)I");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_onControlResolveSegmentDuration, g_clazz.clazz,
        "onControlResolveSegmentDuration", "(Ljava/lang/Object;I)I");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_onControlResolveSegmentUrl, g_clazz.clazz,
        "onControlResolveSegmentUrl", "(Ljava/lang/Object;I)Ljava/lang/String;");

    IJK_FIND_JAVA_STATIC_METHOD(env, g_clazz.jmid_onControlResolveSegmentOfflineMrl, g_clazz.clazz,
        "onControlResolveSegmentOfflineMrl", "(Ljava/lang/Object;I)Ljava/lang/String;");

    ijkmp_global_init();

    FFmpegApi_global_init(env);

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    ijkmp_global_uninit();

    pthread_mutex_destroy(&g_clazz.mutex);
}
