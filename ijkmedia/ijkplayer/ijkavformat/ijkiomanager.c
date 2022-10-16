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
#include "libavutil/log.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CONFIG_MAX_LINE 1024

static int ijkio_manager_alloc(IjkIOManagerContext **ph, void *opaque)
{
    IjkIOManagerContext *h = NULL;

    h = (IjkIOManagerContext *)calloc(1, sizeof(IjkIOManagerContext));
    if (!h)
        return -1;

    h->opaque = opaque;
    h->ijk_ctx_map = ijk_map_create();

    ijkio_application_open(&h->ijkio_app_ctx, opaque);

    pthread_mutex_init(&h->ijkio_app_ctx->mutex, NULL);
    h->ijkio_app_ctx->threadpool_ctx = ijk_threadpool_create(5, 5, 0);
    h->ijkio_app_ctx->cache_info_map = ijk_map_create();
    h->ijkio_app_ctx->fd             = -1;
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

static int tree_destroy(void *parm, int64_t key, void *elem)
{
    IjkCacheTreeInfo *info = elem;
    ijk_av_tree_enumerate(info->root, NULL, NULL, enu_free);
    ijk_av_tree_destroy(info->root);
    free(info);
    return 0;
}

static int enu_save(void *opaque, void *elem) {
    FILE *fp = opaque;
    IjkCacheEntry *entry = elem;
    char string[CONFIG_MAX_LINE] = {0};

    if (entry && fp) {
        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "entry_logical_pos:%lld\n", entry->logical_pos);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "entry_physical_pos:%lld\n", entry->physical_pos);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "entry_size:%lld\n", entry->size);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "entry-info-flush\n");
        fwrite(string, strlen(string), 1, fp);
    }
    return 0;
}

static int ijkio_manager_save_tree_to_file(void *parm, int64_t key, void *elem)
{
    IjkCacheTreeInfo *info = elem;
    FILE *fp = parm;
    char string[CONFIG_MAX_LINE] = {0};
    if (key >= 0 && info) {
        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "tree_index:%lld\n", key);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "tree_physical_init_pos:%lld\n", info->physical_init_pos);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "tree_physical_size:%lld\n", info->physical_size);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "tree_file_size:%lld\n", info->file_size);
        fwrite(string, strlen(string), 1, fp);

        memset(string, 0, CONFIG_MAX_LINE);
        snprintf(string, CONFIG_MAX_LINE, "tree-info-flush\n");
        fwrite(string, strlen(string), 1, fp);

        ijk_av_tree_enumerate(info->root, parm, NULL, enu_save);
    }
    return 0;
}

void ijkio_manager_destroy(IjkIOManagerContext *h)
{
    FILE *map_tree_info_fp = NULL;

    if (h->ijkio_app_ctx) {
        if (h->auto_save_map) {
            map_tree_info_fp = fopen(h->cache_map_path, "w");
            if (map_tree_info_fp) {
                ijk_map_traversal_handle(h->ijkio_app_ctx->cache_info_map, map_tree_info_fp, ijkio_manager_save_tree_to_file);
                fclose(map_tree_info_fp);
            }
        }

        ijk_map_traversal_handle(h->ijkio_app_ctx->cache_info_map, NULL, tree_destroy);
        ijk_map_destroy(h->ijkio_app_ctx->cache_info_map);
        h->ijkio_app_ctx->cache_info_map = NULL;

        if (h->ijkio_app_ctx->threadpool_ctx) {
            ijk_threadpool_destroy(h->ijkio_app_ctx->threadpool_ctx, IJK_IMMEDIATE_SHUTDOWN);
        }

        if (0 != strlen(h->ijkio_app_ctx->cache_file_path)) {
            if (h->ijkio_app_ctx->fd >= 0) {
                close(h->ijkio_app_ctx->fd);
            }
        }
        pthread_mutex_destroy(&h->ijkio_app_ctx->mutex);

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

static int cmp(const void *key, const void *node)
{
    return FFDIFFSIGN(*(const int64_t *)key, ((const IjkCacheEntry *) node)->logical_pos);
}

static void ijkio_manager_parse_cache_info(IjkIOApplicationContext *app_ctx, char *file_path) {
    char string_line[CONFIG_MAX_LINE] = {0};
    char **ptr = (char **)&string_line;
    uint64_t str_len                = 0;
    int tree_index                  = 0;
    int64_t tree_physical_init_pos  = 0;
    int64_t tree_physical_size      = 0;
    int64_t tree_file_size          = 0;
    int64_t entry_logical_pos       = 0;
    int64_t entry_physical_pos      = 0;
    int64_t entry_size              = 0;
    void *cache_info_map            = app_ctx->cache_info_map;
    IjkCacheTreeInfo *cur_tree_info = NULL;
    IjkCacheEntry *cur_entry        = NULL;
    IjkCacheEntry *entry_ret        = NULL;
    struct IjkAVTreeNode *cur_node  = NULL;

    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        return;
    }

    while (!feof(fp)) {
        memset(string_line, 0 , CONFIG_MAX_LINE);
        fgets(string_line, CONFIG_MAX_LINE, fp);

        av_log(NULL, AV_LOG_INFO, "cache config info: %s\n", string_line);

        if (ijk_av_strstart(string_line, "tree_index:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            tree_index = (int)strtol(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "tree_physical_init_pos:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            tree_physical_init_pos = strtoll(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "tree_physical_size:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            tree_physical_size = strtoll(*ptr, NULL, 10);
            app_ctx->last_physical_pos += tree_physical_size;
        } else if (ijk_av_strstart(string_line, "tree_file_size:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            tree_file_size = strtoll(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "tree-info-flush", (const char **)ptr)) {
            cur_tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
            if (cur_tree_info) {
                cur_tree_info->physical_init_pos  = tree_physical_init_pos;
                cur_tree_info->physical_size      = tree_physical_size;
                cur_tree_info->file_size          = tree_file_size;
                ijk_map_put(cache_info_map, tree_index, cur_tree_info);
            } else {
                break;
            }
            tree_index             = 0;
            tree_physical_init_pos = 0;
            tree_physical_size     = 0;
            tree_file_size         = 0;
        } else if (ijk_av_strstart(string_line, "entry_logical_pos:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            entry_logical_pos = strtoll(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "entry_physical_pos:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            entry_physical_pos = strtoll(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "entry_size:", (const char **)ptr)) {
            str_len = strlen(*ptr);
            for (int i = 0; i < str_len; i++) {
                if ((*ptr)[i] < '0' || (*ptr)[i] > '9') {
                    (*ptr)[i] = '\0';
                    break;
                }
            }
            entry_size = strtoll(*ptr, NULL, 10);
        } else if (ijk_av_strstart(string_line, "entry-info-flush", (const char **)ptr)) {
            if (cur_tree_info) {
                cur_entry = calloc(1, sizeof(IjkCacheEntry));
                cur_node  = ijk_av_tree_node_alloc();
                if (!cur_entry || !cur_node) {
                    break;
                }

                cur_entry->logical_pos  = entry_logical_pos;
                cur_entry->physical_pos = entry_physical_pos;
                cur_entry->size         = entry_size;

                entry_ret = ijk_av_tree_insert(&cur_tree_info->root, cur_entry, cmp, &cur_node);
                if (entry_ret && entry_ret != cur_entry) {
                    break;
                }
            }
        }
    }

    fclose(fp);
}

void ijkio_manager_will_share_cache_map(IjkIOManagerContext *h) {
    av_log(NULL, AV_LOG_INFO, "will share cache\n");
    if (!h || !h->ijkio_app_ctx || !strlen(h->cache_map_path)) {
        return;
    }

    pthread_mutex_lock(&h->ijkio_app_ctx->mutex);
    FILE *map_tree_info_fp = fopen(h->cache_map_path, "w");
    if (!map_tree_info_fp) {
        pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
        return;
    }
    h->ijkio_app_ctx->shared = 1;
    ijk_map_traversal_handle(h->ijkio_app_ctx->cache_info_map, map_tree_info_fp, ijkio_manager_save_tree_to_file);
    fclose(map_tree_info_fp);
    if (h->ijkio_app_ctx->fd >= 0) {
        fsync(h->ijkio_app_ctx->fd);
    }
    pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
}

void ijkio_manager_immediate_reconnect(IjkIOManagerContext *h) {
    av_log(NULL, AV_LOG_INFO, "ijkio manager immediate reconnect\n");
    if (!h || !h->ijkio_app_ctx) {
        return;
    }
    h->ijkio_app_ctx->active_reconnect = 1;
}

void ijkio_manager_did_share_cache_map(IjkIOManagerContext *h) {
    av_log(NULL, AV_LOG_INFO, "did share cache\n");
    if (!h || !h->ijkio_app_ctx) {
        return;
    }
    pthread_mutex_lock(&h->ijkio_app_ctx->mutex);
    h->ijkio_app_ctx->shared = 0;
    pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
}

int ijkio_manager_io_open(IjkIOManagerContext *h, const char *url, int flags, IjkAVDictionary **options) {
    int ret = -1;
    int parse_cache_map_file = 0;
    if (!h)
        return ret;

    if (!h->ijkio_app_ctx) {
        return -1;
    }

    IjkAVDictionaryEntry *t = NULL;
    t = ijk_av_dict_get(*options, "cache_file_path", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        strcpy(h->ijkio_app_ctx->cache_file_path, t->value);
    }

    t = ijk_av_dict_get(*options, "cache_map_path", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        strcpy(h->cache_map_path, t->value);

        t = ijk_av_dict_get(*options, "auto_save_map", NULL, IJK_AV_DICT_MATCH_CASE);
        if (t) {
            h->auto_save_map = (int)strtol(t->value, NULL, 10);
        }

        if (h->ijkio_app_ctx->cache_info_map && !ijk_map_size(h->ijkio_app_ctx->cache_info_map)) {
            t = ijk_av_dict_get(*options, "parse_cache_map", NULL, IJK_AV_DICT_MATCH_CASE);
            if (t) {
                parse_cache_map_file = (int)strtol(t->value, NULL, 10);
                if (parse_cache_map_file) {
                    ijkio_manager_parse_cache_info(h->ijkio_app_ctx, h->cache_map_path);
                }
            }
        }
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
