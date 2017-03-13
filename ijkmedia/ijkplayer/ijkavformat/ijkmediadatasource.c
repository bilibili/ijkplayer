/*
 * Copyright (c) 2015 Bilibili
 * Copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifdef __ANDROID__

#include <assert.h>
#include "libavformat/avformat.h"
#include "libavformat/url.h"
#include "libavutil/avstring.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"

#include "ijkavformat/ijkavformat.h"
#include "ijkplayer/ijkavutil/opt.h"

#include "j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.h"
#include "ijksdl/android/ijksdl_android_jni.h"

typedef struct Context {
    AVClass        *class;

    /* options */
    int64_t         logical_pos;
    int64_t         logical_size;

    int64_t         media_data_source_ptr;
    jobject         media_data_source;
    jbyteArray      jbuffer;
    int             jbuffer_capacity;
} Context;

static int ijkmds_open(URLContext *h, const char *arg, int flags, AVDictionary **options)
{
    Context *c = h->priv_data;
    JNIEnv *env = NULL;
    jobject media_data_source = NULL;
    char *final = NULL;

    av_strstart(arg, "ijkmediadatasource:", &arg);

    media_data_source = (jobject) (intptr_t) strtoll(arg, &final, 10);
    if (!media_data_source)
        return AVERROR(EINVAL);

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    c->logical_size = J4AC_IMediaDataSource__getSize(env, media_data_source);
    if (J4A_ExceptionCheck__catchAll(env)) {
        return AVERROR(EINVAL);
    } else if (c->logical_size < 0) {
        h->is_streamed = 1;
        c->logical_size = -1;
    }

    c->media_data_source = (*env)->NewGlobalRef(env, media_data_source);
    if (J4A_ExceptionCheck__catchAll(env) || !c->media_data_source) {
        return AVERROR(ENOMEM);
    }

    return 0;
}

static int ijkmds_close(URLContext *h)
{
    Context *c = h->priv_data;
    JNIEnv *env = NULL;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    J4A_DeleteGlobalRef__p(env, &c->jbuffer);

    if (c->media_data_source) {
        J4AC_IMediaDataSource__close__catchAll(env, c->media_data_source);
        J4A_DeleteGlobalRef__p(env, &c->media_data_source);
    }
    c->media_data_source_ptr = 0;

    return 0;
}

static jobject jbuffer_grow(JNIEnv *env, URLContext *h, int new_capacity) {
    Context *c = h->priv_data;

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

static int ijkmds_read(URLContext *h, unsigned char *buf, int size)
{
    Context    *c = h->priv_data;
    JNIEnv     *env = NULL;
    jbyteArray  jbuffer = NULL;
    jint        ret = 0;

    if (!c->media_data_source) 
        return AVERROR(EINVAL);

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    jbuffer = jbuffer_grow(env, h, size);
    if (!jbuffer)
        return AVERROR(ENOMEM);

    ret = J4AC_IMediaDataSource__readAt(env, c->media_data_source, c->logical_pos, jbuffer, 0, size);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);
    else if (ret < 0)
        return AVERROR_EOF;
    else if (ret == 0)
        return AVERROR(EAGAIN);

    (*env)->GetByteArrayRegion(env, jbuffer, 0, ret, (jbyte*)buf);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);

    c->logical_pos += ret;
    return ret;
}

static int64_t ijkmds_seek(URLContext *h, int64_t pos, int whence)
{
    Context *c = h->priv_data;
    int64_t  ret;
    int64_t  new_logical_pos;
    JNIEnv  *env = NULL;
    jobject  jbuffer = NULL;

    if (!c->media_data_source) 
        return AVERROR(EINVAL);

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        av_log(h, AV_LOG_ERROR, "%s: SDL_JNI_SetupThreadEnv: failed", __func__);
        return AVERROR(EINVAL);
    }

    if (whence == AVSEEK_SIZE) {
        av_log(h, AV_LOG_TRACE, "%s: AVSEEK_SIZE: %"PRId64"\n", __func__, (int64_t)c->logical_size);
        return c->logical_size;
    } else if (whence == SEEK_CUR) {
        av_log(h, AV_LOG_TRACE, "%s: %"PRId64"\n", __func__, pos);
        new_logical_pos = pos + c->logical_pos;
    } else if (whence == SEEK_SET){
        av_log(h, AV_LOG_TRACE, "%s: %"PRId64"\n", __func__, pos);
        new_logical_pos = pos;
    } else {
        return AVERROR(EINVAL);
    }
    if (new_logical_pos < 0)
        return AVERROR(EINVAL);

    jbuffer = jbuffer_grow(env, h, 0);
    if (!jbuffer)
        return AVERROR(ENOMEM);

    ret = J4AC_IMediaDataSource__readAt(env, c->media_data_source, new_logical_pos, jbuffer, 0, 0);
    if (J4A_ExceptionCheck__catchAll(env))
        return AVERROR(EIO);
    else if (ret < 0)
        return AVERROR_EOF;

    c->logical_pos = new_logical_pos;
    return c->logical_pos;
}

#define OFFSET(x) offsetof(Context, x)
#define D AV_OPT_FLAG_DECODING_PARAM

static const AVOption options[] = {
    { NULL }
};

#undef D
#undef OFFSET

static const AVClass ijkmediadatasource_context_class = {
    .class_name = "IjkMediaDataSource",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

URLProtocol ijkimp_ff_ijkmediadatasource_protocol = {
    .name                = "ijkmediadatasource",
    .url_open2           = ijkmds_open,
    .url_read            = ijkmds_read,
    .url_seek            = ijkmds_seek,
    .url_close           = ijkmds_close,
    .priv_data_size      = sizeof(Context),
    .priv_data_class     = &ijkmediadatasource_context_class,
};

#endif
