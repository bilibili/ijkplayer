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

#include "ijkiomanager.h"
#include "ijkioprotocol.h"
#include "ijkplayer/ijkavutil/ijkutils.h"
#include "ijkplayer/ijkavutil/ijktree.h"
#include "ijkplayer/ijkavutil/ijkstl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int ijkio_manager_alloc(IjkIOManagerContext **ph, void *opaque)
{
    IjkIOManagerContext *h = NULL;

    h = (IjkIOManagerContext *)calloc(1, sizeof(IjkIOManagerContext));
    if (!h)
        return -1;

    h->opaque = opaque;
    h->ijk_ctx_map = ijk_map_create();

    ijkio_application_open(&h->ijkio_app_ctx, opaque);

    h->ijkio_app_ctx->threadpool_ctx = ijk_threadpool_create(5, 5, 0);
    h->ijkio_app_ctx->cache_info_map = ijk_map_create();
    *ph = h;
    return 0;
}

int ijkio_manager_create(IjkIOManagerContext **ph, void *opaque)
{
    return ijkio_manager_alloc(ph, opaque);
}

static int enu_free(void *opaque, void *elem)
{
    free(elem);
    return 0;
}

static int tree_destroy(void *elem)
{
    IjkCacheTreeInfo *info = elem;
    ijk_av_tree_enumerate(info->root, NULL, NULL, enu_free);
    ijk_av_tree_destroy(info->root);
    free(info);
    return 0;
}

void ijkio_manager_destroy(IjkIOManagerContext *h)
{
    if (h->ijkio_app_ctx) {
        ijk_map_traversal_handle(h->ijkio_app_ctx->cache_info_map, tree_destroy);
        ijk_map_destroy(h->ijkio_app_ctx->cache_info_map);
        h->ijkio_app_ctx->cache_info_map = NULL;

        if (h->ijkio_app_ctx->threadpool_ctx) {
            ijk_threadpool_destroy(h->ijkio_app_ctx->threadpool_ctx, IJK_IMMEDIATE_SHUTDOWN);
        }

        if (0 != strlen(h->ijkio_app_ctx->cache_file_path)) {
            remove(h->ijkio_app_ctx->cache_file_path);
        }

        ijkio_application_closep(&h->ijkio_app_ctx);
    }

    ijk_map_destroy(h->ijk_ctx_map);
    h->ijk_ctx_map = NULL;

    free(h);
}

void ijkio_manager_destroyp(IjkIOManagerContext **ph)
{
    if (!ph || !*ph)
        return;

    ijkio_manager_destroy(*ph);
    *ph = NULL;
}

int ijkio_manager_set_callback(IjkIOManagerContext *h, void *callback) {
    if (!h)
        return -1;

    h->ijkio_app_ctx->func_ijkio_on_app_event = callback;

    return 0;
}



static void ijkio_manager_set_all_ctx_pause(IjkIOManagerContext *h) {
    IjkURLContext *url_ctx;
    int size = ijk_map_size(h->ijk_ctx_map);

    for(int i = 0; i < size; i++) {
        url_ctx = ijk_map_index_get(h->ijk_ctx_map, i);
        if (url_ctx == NULL || url_ctx->prot == NULL)
            break;

        if (url_ctx->prot->url_pause)
            url_ctx->prot->url_pause(url_ctx);
        url_ctx->state = IJKURL_PAUSED;
    }
}

int ijkio_manager_io_open(IjkIOManagerContext *h, const char *url, int flags, IjkAVDictionary **options) {
    int ret = -1;
    if (!h)
        return ret;
    IjkAVDictionaryEntry *t = NULL;
    t = ijk_av_dict_get(*options, "cache_file_path", t, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        strcpy(h->ijkio_app_ctx->cache_file_path, t->value);
    }
    if (h->ijkio_app_ctx == NULL) {
        return -1;
    }

    h->ijkio_app_ctx->ijkio_interrupt_callback = h->ijkio_interrupt_callback;

    IjkURLContext *inner = NULL;
    ijkio_alloc_url(&inner, url);
    if (inner) {
        inner->ijkio_app_ctx = h->ijkio_app_ctx;
        if (h->ijk_ctx_map) {
            ijkio_manager_set_all_ctx_pause(h);
            inner->state = IJKURL_STARTED;
            ijk_map_put(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx, inner);
        }
        ret = inner->prot->url_open2(inner, url, flags, options);
        if (ret != 0)
            goto fail;

        return ret;
    }

fail:
    if (inner) {
        if (inner->prot && inner->prot->url_close)
            ret = inner->prot->url_close(inner);

        if (h->ijk_ctx_map) {
            ijk_map_remove(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx);
        }
        ijk_av_freep(&inner->priv_data);
        ijk_av_freep(&inner);
    }
    return -1;
}

int ijkio_manager_io_read(IjkIOManagerContext *h, unsigned char *buf, int size) {
    int ret = -1;
    if (!h)
        return ret;

    IjkURLContext *inner = ijk_map_get(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx);
    if (inner && inner->prot && inner->prot->url_read) {
        if (inner->state == IJKURL_PAUSED) {
            if (inner->prot->url_resume) {
                ret = inner->prot->url_resume(inner);
                if (ret != 0) {
                    return ret;
                }
            }
            inner->state = IJKURL_STARTED;
        }
        ret = inner->prot->url_read(inner, buf, size);
    }

    return ret;
}

int64_t ijkio_manager_io_seek(IjkIOManagerContext *h, int64_t offset, int whence) {
    int64_t ret = -1;

    if (!h)
        return ret;

    IjkURLContext *inner = ijk_map_get(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx);
    if (inner && inner->prot && inner->prot->url_seek) {
        if (inner->state == IJKURL_PAUSED) {
            if (inner->prot->url_resume) {
                ret = (int64_t)inner->prot->url_resume(inner);
                if (ret < 0) {
                    return ret;
                }
            }
            inner->state = IJKURL_STARTED;
        }
        ret = inner->prot->url_seek(inner, offset, whence & ~IJKAVSEEK_FORCE);
    }

    return ret;
}

int ijkio_manager_io_close(IjkIOManagerContext *h) {
    int ret = -1;
    if (!h)
        return ret;

    IjkURLContext *inner = ijk_map_get(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx);
    if (inner) {
        if (inner->prot && inner->prot->url_close) {
            ret = inner->prot->url_close(inner);
        }
        ijk_map_remove(h->ijk_ctx_map, (int64_t)(intptr_t)h->cur_ffmpeg_ctx);
        ijk_av_freep(&inner->priv_data);
        ijk_av_freep(&inner);
    }

    return ret;
}
