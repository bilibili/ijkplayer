/*****************************************************************************
 * ijkplayer_jni.c
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

#include <assert.h>
#include <string.h>
#include <jni.h>
#include "helpers/loghelper.h"
#include "helpers/JNIHelp.h"
#include "ijkplayer.h"

#define JNI_MODULE_PACKAGE      "tv/danmaku/ijk/media/player"
#define JNI_CLASS_IJKPLAYER     "tv/danmaku/ijk/media/player/IjkMediaPlayer"


typedef struct player_fields_t {
    jclass      clazz;

    jfieldID    context;
    jfieldID    surface_texture;

    jmethodID   post_event;
} player_fields_t;
static player_fields_t g_jni_player_fields;


inline static int jni_get_int_fields(JNIEnv* env, jobject thiz, jfieldID field)
{
    return (*env)->GetIntField(env, thiz, field);
}

static IjkMediaPlayer *get_media_player(JNIEnv* env, jobject thiz)
{
    // FIXME: lock ref count
    IjkMediaPlayer *mp = (IjkMediaPlayer *)jni_get_int_fields(env, thiz, g_jni_player_fields.context);
    return mp;
}

static void
IjkMediaPlayer_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values) {
    //IjkMediaPlayer *mp = get_media_player(env, thiz);
    //ijkmp_set_data_source(mp, path);
}

static void
IjkMediaPlayer_setVideoSurface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_prepareAsync(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_start(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_stop(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static int
IjkMediaPlayer_getVideoWidth(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
    return 0;
}

static int
IjkMediaPlayer_getVideoHeight(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
    return 0;
}

static void
IjkMediaPlayer_seekTo(JNIEnv *env, jobject thiz, int msec)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_pause(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static jboolean
IjkMediaPlayer_isPlaying(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
    return JNI_FALSE;
}

static int
IjkMediaPlayer_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
    return 0;
}

static int
IjkMediaPlayer_getDuration(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
    return 0;
}

static void
IjkMediaPlayer_release(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_reset(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_setAudioStreamType(JNIEnv *env, jobject thiz, int streamtype)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_native_init(JNIEnv *env)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_native_setup(JNIEnv *env, jobject thiz, jobject weak_this)
{
    // FIXME: implement
}

static void
IjkMediaPlayer_native_finalize(JNIEnv *env, jobject thiz)
{
    // FIXME: implement
}

// ----------------------------------------------------------------------------

static JNINativeMethod g_ijkplayer_methods[] = {
    {
        "_setDataSource",
        "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V",
        (void *) IjkMediaPlayer_setDataSourceAndHeaders
    },
    { "_setVideoSurface", "(Landroid/view/Surface;)V", (void *) IjkMediaPlayer_setVideoSurface },
    { "prepareAsync", "()V", (void *) IjkMediaPlayer_prepareAsync },
    { "_start", "()V", (void *) IjkMediaPlayer_start },
    { "_stop", "()V", (void *) IjkMediaPlayer_stop },
    { "getVideoWidth", "()I", (void *) IjkMediaPlayer_getVideoWidth },
    { "getVideoHeight", "()I", (void *) IjkMediaPlayer_getVideoHeight },
    { "seekTo", "(I)V", (void *) IjkMediaPlayer_seekTo },
    { "_pause", "()V", (void *) IjkMediaPlayer_pause },
    { "isPlaying", "()Z", (void *) IjkMediaPlayer_isPlaying },
    { "getCurrentPosition", "()I", (void *) IjkMediaPlayer_getCurrentPosition },
    { "getDuration", "()I", (void *) IjkMediaPlayer_getDuration },
    { "_release", "()V", (void *) IjkMediaPlayer_release },
    { "_reset", "()V", (void *) IjkMediaPlayer_reset },
    { "setAudioStreamType", "(I)V", (void *) IjkMediaPlayer_setAudioStreamType },
    { "native_init", "()V", (void *) IjkMediaPlayer_native_init },
    { "native_setup", "(Ljava/lang/Object;)V", (void *) IjkMediaPlayer_native_setup },
    { "native_finalize", "()V", (void *) IjkMediaPlayer_native_finalize },
};

void jni_init(JavaVM *vm, JNIEnv* env) {
    g_jni_player_fields.clazz = (*env)->FindClass(env, JNI_CLASS_IJKPLAYER);

    (*env)->RegisterNatives(env, g_jni_player_fields.clazz, g_ijkplayer_methods, NELEM(g_ijkplayer_methods));
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    jni_init(vm, env);
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
}
