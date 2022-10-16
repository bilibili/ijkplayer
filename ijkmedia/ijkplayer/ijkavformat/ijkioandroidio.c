/*
 * Copyright (c) 2016 Bilibili
 * Copyright (c) 2016 Raymond Zheng <raymondzheng1412@gmail.com>
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
#include "ijkiourl.h"
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"

#include "ijkavformat/ijkavformat.h"
#include "ijkplayer/ijkavutil/opt.h"
#include "ijkplayer/ijkavutil/ijkutils.h"

#include "j4a/class/tv/danmaku/ijk/media/player/misc/IAndroidIO.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include <assert.h>

typedef struct IjkIOAndroidioContext {
    jobject         ijkio_androidio;
    jbyteArray      jbuffer;
    int             jbuffer_capacity;
    URLContext *inner;
} IjkIOAndroidioContext;

static int ijkio_androidio_open(IjkURLContext *h, const char *url, int flags, IjkAVDictionary **options) {
    IjkIOAndroidioContext *c= h->priv_data;
    JNIEnv *env = NULL;
    jobject ijkio_androidio = NULL;
    char *final = NULL;

    if (!c)
        return -1;

    av_strstart(url, "androidio:", &url);

    IjkAVDictionaryEntry *t = NULL;
    t = ijk_av_dict_get(*options, "androidio-inject-callback", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        ijkio_androidio = (jobject) (intptr_t) strtoll(t->value, &final, 10);
    } else {
        return -1;
    }

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    if (!ijkio_androidio)
        return AVERROR(EINVAL);

    c->ijkio_androidio = (*env)->NewGlobalRef(env, ijkio_androidio);
    if (J4A_ExceptionCheck__catchAll(env) || !c->ijkio_androidio) {
        return AVERROR(ENOMEM);
    }

    jstring urlString = NULL;
    urlString = (*env)->NewStringUTF(env, url);

    jint ret = J4AC_IAndroidIO__open(env, c->ijkio_androidio, urlString);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return AVERROR(EINVAL);
    } else if (ret < 0) {
        return ret;
    }

    return 0;
}

static jobject jbuffer_grow(JNIEnv *env, IjkURLContext *h, int new_capacity) {
    IjkIOAndroidioContext *c = h->priv_data;
    if (!c)
        return NULL;

    if (c->jbuffer && c->jbuffer_capacity >= new_capacity)
        return c->jbuffer;

    new_capacity = FFMAX(new_capacity, c->jbuffer_capacity * 2);

    J4A_DeleteGlobalRef__p(env, &c->jbuffer);
    c->jbuffer_capacity = 0;

    c->jbuffer = J4A_NewByteArray__asGlobalRef__catchAll(env, new_capacity);
    if (J4A_ExceptionCheck__catchAll(env) || !c->jbuffer) {
        c->jbuffer = NULL;
        return NULL;
    }

    c->jbuffer_capacity = new_capacity;
    return c->jbuffer;
}

static int ijkio_androidio_read(IjkURLContext *h, unsigned char *buf, int size) {
    IjkIOAndroidioContext    *c = h->priv_data;
    JNIEnv     *env = NULL;
    jbyteArray  jbuffer = NULL;
    jint        ret = 0;

    if (!c || !c->ijkio_androidio)
        return AVERROR(EINVAL);

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    jbuffer = jbuffer_grow(env, h, size);
    if (!jbuffer)
        return AVERROR(ENOMEM);

    ret = J4AC_IAndroidIO__read(env, c->ijkio_androidio, jbuffer, size);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);
    else if (ret < 0)
        return AVERROR_EOF;
    else if (ret == 0)
        return AVERROR(EAGAIN);

    (*env)->GetByteArrayRegion(env, jbuffer, 0, ret, (jbyte*)buf);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);

    return ret;
}

static int64_t ijkio_androidio_seek(IjkURLContext *h, int64_t offset, int whence) {
    IjkIOAndroidioContext *c = h->priv_data;
    int64_t  ret;
    JNIEnv  *env = NULL;

    if (!c || !c->ijkio_androidio)
        return AVERROR(EINVAL);

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    ret = J4AC_IAndroidIO__seek(env, c->ijkio_androidio, offset, whence);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);

    return ret;
}

static int ijkio_androidio_close(IjkURLContext *h) {
    IjkIOAndroidioContext *c = h->priv_data;
    JNIEnv *env = NULL;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    if (!c || !c->ijkio_androidio)
        return AVERROR(EINVAL);

    J4A_DeleteGlobalRef__p(env, &c->jbuffer);

    if (c->ijkio_androidio) {
        J4AC_IAndroidIO__close__catchAll(env, c->ijkio_androidio);
        J4A_DeleteGlobalRef__p(env, &c->ijkio_androidio);
    }

    return 0;
}

IjkURLProtocol ijkio_androidio_protocol = {
    .name                = "ijkioandroidio",
    .url_open2           = ijkio_androidio_open,
    .url_read            = ijkio_androidio_read,
    .url_seek            = ijkio_androidio_seek,
    .url_close           = ijkio_androidio_close,
    .priv_data_size      = sizeof(IjkIOAndroidioContext),
};
