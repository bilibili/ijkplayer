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
#include "ijkioprotocol.h"
#include "ijkioapplication.h"
#include "ijkplayer/ijkavutil/ijktree.h"
#include "ijkplayer/ijkavutil/ijkutils.h"
#include "ijkplayer/ijkavutil/ijkthreadpool.h"
#include "ijkplayer/ijkavutil/ijkstl.h"
#include "libavutil/log.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#define DEFAULT_CACHE_MAX_CAPACITY            (512 * 1024 * 1024)
#define DEFAULT_CACHE_FILE_FORWARDS_CAPACITY  (8 * 1024 * 1024)
#   ifndef O_BINARY
#       define O_BINARY 0
#   endif
#define FILE_RW_ERROR  (-100)

typedef struct IjkIOCacheContext {
    char *cache_file_path;
    int fd;
    IjkCacheTreeInfo *tree_info;
    int64_t logical_size;
    int64_t read_logical_pos;
    int64_t read_inner_pos;
    int64_t file_logical_pos;
    int64_t cache_physical_pos;
    int64_t file_inner_pos;
    int64_t file_logical_end;
    int64_t cache_max_capacity;
    int64_t cache_file_forwards_capacity;
    int cache_file_close;
    int io_eof_reached;
    int io_error;
    int inner_io_error;
    int read_file_inner_error;
    int file_handle_retry_count;
    int file_error_count;
    int seek_request;
    int seek_completed;
    int seek_whence;
    int64_t seek_pos;
    int64_t seek_ret;

    int cur_file_no;
    void *cache_info_map;
    int64_t *last_physical_pos;
    int64_t *cache_count_bytes;

    pthread_cond_t     cond_wakeup_main;
    pthread_cond_t     cond_wakeup_file_background;
    pthread_cond_t     cond_wakeup_exit;
    pthread_cond_t     cond_wakeup_write_file_exit;
    pthread_mutex_t    file_mutex;
    int                abort_request;
    IjkAVIOInterruptCB *ijkio_interrupt_callback;
    int task_is_running;

    IjkURLContext *inner;
    IjkThreadPoolContext *threadpool_ctx;
    IjkIOApplicationContext *ijkio_app_ctx;
    int async_open;
    IjkAVDictionary *inner_options;
    char inner_url[4096];
    int inner_flags;
    int only_read_file;
} IjkIOCacheContext;

static int cmp(const void *key, const void *node)
{
    return FFDIFFSIGN(*(const int64_t *)key, ((const IjkCacheEntry *) node)->logical_pos);
}

static void call_inject_statistic(IjkURLContext *h)
{
    IjkIOCacheContext *c = h->priv_data;

    if (c->ijkio_app_ctx) {
        IjkIOAppCacheStatistic statistic = {0};
        statistic.cache_physical_pos  = c->cache_physical_pos;
        statistic.cache_file_forwards = c->file_logical_pos - c->read_logical_pos;
        statistic.cache_file_pos      = c->file_logical_pos;
        statistic.cache_count_bytes   = *c->cache_count_bytes;
        statistic.logical_file_size   = c->logical_size;
        ijkio_application_on_cache_statistic(c->ijkio_app_ctx, &statistic);
    }
}

static int ijkio_cache_check_interrupt(IjkURLContext *h)
{
    IjkIOCacheContext *c = h->priv_data;
    if (!c)
        return 1;

    if (c->abort_request)
        return 1;

    if (c->ijkio_interrupt_callback && c->ijkio_interrupt_callback->callback &&
                    c->ijkio_interrupt_callback->callback(c->ijkio_interrupt_callback->opaque)) {
        c->abort_request = 1;
    }

    if (c->abort_request)
        return 1;

    return c->abort_request;
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

static int ijkio_cache_file_error(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;

    av_log(NULL, AV_LOG_WARNING, "ijkio_cache_file_error\n");
    if (c && c->file_handle_retry_count > 3) {
        pthread_mutex_lock(&h->ijkio_app_ctx->mutex);
        c->file_error_count++;
        if (!c->ijkio_app_ctx->shared) {
            ijk_map_traversal_handle(c->cache_info_map, NULL, tree_destroy);
            ijk_map_clear(c->cache_info_map);
            c->tree_info = NULL;
            *c->last_physical_pos    = 0;
            c->cache_physical_pos    = 0;
            c->file_inner_pos        = 0;
            c->io_eof_reached        = 0;
            c->file_logical_pos      = c->read_logical_pos;
            close(c->fd);
            c->fd = -1;
            c->ijkio_app_ctx->fd = -1;
            if (c->file_error_count > 3) {
                c->cache_file_close = 1;
                remove(c->cache_file_path);
                av_log(NULL, AV_LOG_WARNING, "ijkio_cache_file_error will remove file\n");
                goto fail;
            }
            c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0600);
            c->ijkio_app_ctx->fd = c->fd;
            if (c->fd >= 0) {
                c->file_handle_retry_count = 0;
                c->tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
                if (!c->tree_info) {
                    c->cache_file_close = 1;
                    goto fail;
                }
                ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);
            } else {
                av_log(NULL, AV_LOG_WARNING, "ijkio_cache_file_error will cache_file_close\n");
                c->cache_file_close = 1;
                goto fail;
            }
        }
        pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
    }

    return 0;

fail:
    pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
    return FILE_RW_ERROR;
}

static int64_t ijkio_cache_file_overrang(IjkURLContext *h, int64_t *cur_pos, int size) {
    IjkIOCacheContext *c = h->priv_data;
    av_log(NULL, AV_LOG_WARNING, "ijkio_cache_file_overrang will flush file\n");

    pthread_mutex_lock(&h->ijkio_app_ctx->mutex);

    if (!c->ijkio_app_ctx->shared) {
        ijk_map_remove(c->cache_info_map, (int64_t)c->cur_file_no);
        ijk_map_traversal_handle(c->cache_info_map, NULL, tree_destroy);
        ijk_map_clear(c->cache_info_map);
        memset(c->tree_info, 0, sizeof(IjkCacheTreeInfo));
        ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);
        *c->last_physical_pos    = 0;
        c->cache_physical_pos    = 0;
        c->io_eof_reached        = 0;
        c->file_logical_pos      = c->read_logical_pos;
        *cur_pos = lseek(c->fd, 0, SEEK_SET);
        if (*cur_pos < 0) {
            goto fail;
        }
    } else {
        goto fail;
    }

    pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
    return c->cache_max_capacity;

fail:
    pthread_mutex_unlock(&h->ijkio_app_ctx->mutex);
    return FILE_RW_ERROR;
}

static int64_t add_entry(IjkURLContext *h, const unsigned char *buf, int size)
{
    IjkIOCacheContext *c= h->priv_data;
    int64_t pos = -1;
    int64_t ret = 0;
    IjkCacheEntry *entry = NULL, *next[2] = {NULL, NULL};
    IjkCacheEntry *entry_ret = NULL;
    struct IjkAVTreeNode *node = NULL;
    int64_t free_space = 0;

    //FIXME avoid lseek
    pos = lseek(c->fd, *c->last_physical_pos, SEEK_SET);

    if (pos < 0) {
        c->file_handle_retry_count++;
        return ijkio_cache_file_error(h);
    } else {
        c->cache_physical_pos = pos;
        *c->last_physical_pos = pos;
    }

    if (pos + size >= c->cache_max_capacity) {
        free_space = ijkio_cache_file_overrang(h, &pos, size);
        if (free_space < size) {
            c->cache_file_close = 1;
            return FILE_RW_ERROR;
        }
        if (pos < 0) {
            c->file_handle_retry_count++;
            return ijkio_cache_file_error(h);
        }
        if (free_space == c->cache_max_capacity)
            return 0;
    }

    ret = write(c->fd, buf, size);
    if (ret < 0) {
        c->file_handle_retry_count++;
        return ijkio_cache_file_error(h);
    } else {
        c->file_handle_retry_count = 0;
    }

    c->cache_physical_pos       += ret;
    *c->last_physical_pos       += ret;
    c->tree_info->physical_size += ret;

    entry = ijk_av_tree_find(c->tree_info->root, &c->file_logical_pos, cmp, (void**)next);

    if (!entry)
        entry = next[0];

    if (!entry ||
        entry->logical_pos  + entry->size != c->file_logical_pos ||
        entry->physical_pos + entry->size != pos) {
        entry = malloc(sizeof(*entry));
        node = ijk_av_tree_node_alloc();
        if (!entry || !node) {
            ret = IJKAVERROR(ENOMEM);
            goto fail;
        }
        entry->logical_pos = c->file_logical_pos;
        entry->physical_pos = pos;
        entry->size = ret;

        entry_ret = ijk_av_tree_insert(&c->tree_info->root, entry, cmp, &node);
        if (entry_ret && entry_ret != entry) {
            ret = -1;
            av_log(NULL, AV_LOG_ERROR, "av_tree_insert failed\n");
            goto fail;
        }
    } else
        entry->size += ret;

    return ret;
fail:
    //we could truncate the file to pos here if pos >=0 but ftruncate isn't available in VS so
    //for simplicty we just leave the file a bit larger
    free(entry);
    free(node);
    return ret;
}

static int wrapped_file_read(IjkURLContext *h, void *dst, int size)
{
    IjkIOCacheContext *c   = h->priv_data;
    int ret;

    ret = (int)read(c->fd, dst, size);
    c->read_file_inner_error = ret < 0 ? ret : 0;
    return ret;
}

static int wrapped_url_read(IjkURLContext *h, void *dst, int size)
{
    IjkIOCacheContext *c   = h->priv_data;
    int ret;

    ret = c->inner->prot->url_read(c->inner, dst, size);

    if (ret > 0)
        *c->cache_count_bytes += ret;

    c->inner_io_error = ret < 0 ? ret : 0;

    return ret;
}

static int64_t ijkio_cache_ffurl_size(IjkURLContext *h) {
    int64_t pos, size;
    IjkIOCacheContext *c= ((IjkURLContext *)h)->priv_data;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);
    size = c->inner->prot->url_seek(c->inner, 0, IJKAVSEEK_SIZE);
    if (size < 0) {
        pos = c->inner->prot->url_seek(c->inner, 0, SEEK_CUR);
        if ((size = c->inner->prot->url_seek(c->inner, -1, SEEK_END)) < 0)
            return size;
        size++;
        c->inner->prot->url_seek(c->inner, pos, SEEK_SET);
    }
    return size;
}

static int ijkio_cache_io_open(IjkURLContext *h, const char *url, int flags, IjkAVDictionary **options) {
    int ret = 0;
    IjkIOCacheContext *c= h->priv_data;
    ret = c->inner->prot->url_open2(c->inner, url, flags, options);
    if (ret != 0) {
        return ret;
    } else {
        c->logical_size = ijkio_cache_ffurl_size(h);
        if (c->tree_info && !c->cache_file_close) {
            c->tree_info->file_size = c->logical_size;
        }
    }

    call_inject_statistic(h);
    return ret;
}

static int64_t ijkio_cache_write_file(IjkURLContext *h) {
    IjkIOCacheContext *c= h->priv_data;
    int64_t r;
    unsigned char buf[4096] = {0};
    int to_read = 4096;
    int64_t to_copy = (int64_t)to_read;

    IjkCacheEntry *root = NULL ,*l_entry = NULL, *r_entry = NULL, *next[2] = {NULL, NULL};

    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    root = ijk_av_tree_find(c->tree_info->root, &c->file_logical_pos, cmp, (void**)next);

    if (!root)
        l_entry = next[0];

    if (l_entry) {
        int64_t in_block_pos = c->file_logical_pos - l_entry->logical_pos;
        assert(l_entry->logical_pos <= c->file_logical_pos);
        if (in_block_pos < l_entry->size) {
            c->file_logical_pos = l_entry->logical_pos + l_entry->size;
        }
    } else if (root) {
        int64_t in_block_pos = c->file_logical_pos - root->logical_pos;
        assert(root->logical_pos <= c->file_logical_pos);
        if (in_block_pos < root->size) {
            c->file_logical_pos = root->logical_pos + root->size;
        }
    }

    r_entry = next[1];

    if (r_entry) {
        to_copy = r_entry->logical_pos - c->file_logical_pos;
        to_copy = FFMIN(to_copy, to_read);
    }

    if (to_copy == 0) {
        return 0;
    }

    if (c->file_logical_end > 0 && c->file_logical_pos == c->file_logical_end) {
        c->io_eof_reached = 1;
        return 0;
    }

    if (c->file_logical_pos >= c->logical_size) {
        c->io_eof_reached = 1;
        return 0;
    }
    if (c->file_logical_pos != c->file_inner_pos) {
        if (c->async_open > 0) {
            r = ijkio_cache_io_open(h, c->inner_url, c->inner_flags, &c->inner_options);
            if (r != 0) {
                c->io_eof_reached = 1;
                c->io_error = (int)r;
                return r;
            }
            c->async_open = 0;
        }

        r = c->inner->prot->url_seek(c->inner, c->file_logical_pos, SEEK_SET);

        if (r < 0) {
            c->io_eof_reached = 1;
            if (c->file_logical_end == c->file_logical_pos) {
                c->file_inner_pos = c->file_logical_end;
            }
            return r;
        }
        c->file_inner_pos = r;
    }
    if (c->async_open > 0) {
        r = ijkio_cache_io_open(h, c->inner_url, c->inner_flags, &c->inner_options);
        if (r != 0) {
            c->io_eof_reached = 1;
            c->io_error = (int)r;
            return r;
        }
        c->async_open = 0;
    }
    r = c->inner->prot->url_read(c->inner, buf, (int)to_copy);
    if (r == 0 && to_copy > 0) {
        c->file_logical_end = c->file_logical_pos;
    }
    if (r <= 0) {
        c->io_eof_reached = 1;
        c->io_error = (int)r;
        return r;
    }
    *c->cache_count_bytes += r;
    c->file_inner_pos += r;

    pthread_mutex_lock(&c->file_mutex);
    r = add_entry(h, buf, (int)r);

    if (r > 0) {
        c->file_logical_pos += r;
        pthread_cond_signal(&c->cond_wakeup_file_background);
    }
    pthread_mutex_unlock(&c->file_mutex);

    return r;
}

static void ijkio_cache_task(void *h, void *r) {
    IjkIOCacheContext *c= ((IjkURLContext *)h)->priv_data;
    c->task_is_running = 1;
    int64_t ret = 0;

    while(1) {
        if (c->cache_file_close) {
            break;
        }
        if (ijkio_cache_check_interrupt(h)) {
            c->io_eof_reached   = 1;
            c->io_error         = IJKAVERROR_EXIT;
            break;
        }

        if (c->seek_request) {
            pthread_mutex_lock(&c->file_mutex);
            c->io_eof_reached    = 0;
            c->io_error          = 0;
            c->seek_completed    = 1;
            c->seek_request      = 0;
            c->read_logical_pos  = c->seek_pos;
            c->file_logical_pos  = c->seek_pos;
            c->seek_ret          = c->seek_pos;
            pthread_cond_signal(&c->cond_wakeup_main);
            pthread_mutex_unlock(&c->file_mutex);
        }

        if (((c->file_logical_pos - c->read_logical_pos > c->cache_file_forwards_capacity)
            || c->io_eof_reached)) {
            pthread_mutex_lock(&c->file_mutex);
            pthread_cond_signal(&c->cond_wakeup_main);
            pthread_cond_wait(&c->cond_wakeup_file_background, &c->file_mutex);
            pthread_mutex_unlock(&c->file_mutex);
        } else {
            ret = ijkio_cache_write_file(h);
            if (ret > 0) {
                pthread_mutex_lock(&c->file_mutex);
                pthread_cond_signal(&c->cond_wakeup_main);
                pthread_mutex_unlock(&c->file_mutex);
            } else if (ret == FILE_RW_ERROR) {
                break;
            }
        }

        call_inject_statistic(h);
    }
    pthread_mutex_lock(&c->file_mutex);
    c->task_is_running = 0;
    pthread_cond_signal(&c->cond_wakeup_main);
    pthread_cond_signal(&c->cond_wakeup_exit);
    pthread_mutex_unlock(&c->file_mutex);
}

static int ijkio_cache_open(IjkURLContext *h, const char *url, int flags, IjkAVDictionary **options) {
    IjkIOCacheContext *c= h->priv_data;
    int ret = 0;
    int64_t cur_exist_file_size = 0;
    if (!c)
        return IJKAVERROR(ENOSYS);

    c->ijkio_app_ctx = h->ijkio_app_ctx;
    if (c->ijkio_app_ctx == NULL) {
        return -1;
    }

    c->async_open = 0;
    c->ijkio_interrupt_callback = h->ijkio_app_ctx->ijkio_interrupt_callback;
    c->cache_file_forwards_capacity = 0;

    ijk_av_strstart(url, "cache:", &url);
    c->cache_max_capacity = DEFAULT_CACHE_MAX_CAPACITY;

    IjkAVDictionaryEntry *t = NULL;
    t = ijk_av_dict_get(*options, "cache_max_capacity", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->cache_max_capacity = strtoll(t->value, NULL, 10);
    }

    t = ijk_av_dict_get(*options, "cache_file_forwards_capacity", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->cache_file_forwards_capacity = strtoll(t->value, NULL, 10);
    }

    t = ijk_av_dict_get(*options, "cache_file_close", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->cache_file_close = (int)strtol(t->value, NULL, 10);
        c->cache_file_close = c->cache_file_close != 0 ? 1 : 0;
    }

    t = ijk_av_dict_get(*options, "cur_file_no", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->cur_file_no = (int)strtol(t->value, NULL, 10);
    }

    t = ijk_av_dict_get(*options, "only_read_file", NULL, IJK_AV_DICT_MATCH_CASE);
    if (t) {
        c->only_read_file = (int)strtol(t->value, NULL, 10);
        if (c->only_read_file) {
            c->cache_file_forwards_capacity = 0;
        }
    }

    c->cache_file_path = c->ijkio_app_ctx->cache_file_path;

    if (c->cache_file_path == NULL || 0 == strlen(c->cache_file_path)) {
        c->cache_file_close = 1;
    }

    c->threadpool_ctx       = c->ijkio_app_ctx->threadpool_ctx;
    c->cache_info_map       = c->ijkio_app_ctx->cache_info_map;
    c->last_physical_pos    = &c->ijkio_app_ctx->last_physical_pos;
    c->cache_count_bytes    = &c->ijkio_app_ctx->cache_count_bytes;
    if (!c->last_physical_pos || !c->threadpool_ctx || !c->cache_info_map) {
        return -1;
    }

    if (!c->cache_file_close) {
        do {
            if (c->ijkio_app_ctx->fd >= 0) {
                c->fd = c->ijkio_app_ctx->fd;
            } else {
                if (ijk_map_size(c->cache_info_map) > 0) {
                    av_log(NULL, AV_LOG_INFO, "ijkio cache will use the data that already exists\n");
                    c->fd = open(c->cache_file_path, O_RDWR | O_BINARY, 0600);
                    c->async_open = 1;
                    cur_exist_file_size = lseek(c->fd, 0, SEEK_END);
                    if (cur_exist_file_size < *c->last_physical_pos) {
                        av_log(NULL, AV_LOG_WARNING, "ijkio cache exist is error, will delete last_physical_pos = %lld, cur_exist_file_size = %lld\n", *c->last_physical_pos, cur_exist_file_size);
                        ijk_map_traversal_handle(c->cache_info_map, NULL, tree_destroy);
                        ijk_map_clear(c->cache_info_map);
                        *c->last_physical_pos    = 0;
                        c->cache_physical_pos    = 0;
                    }
                } else {
                    c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0600);
                }
                c->ijkio_app_ctx->fd = c->fd;
            }
            if (c->fd < 0) {
                c->cache_file_close = 1;
                break;
            }

            int64_t seek_ret = lseek(c->fd, *c->last_physical_pos, SEEK_SET);
            if (seek_ret < 0) {
                c->cache_file_close = 1;
                close(c->fd);
                c->fd = -1;
                c->ijkio_app_ctx->fd = -1;
                break;
            } else {
                c->cache_physical_pos = *c->last_physical_pos;
            }

            c->tree_info = ijk_map_get(c->cache_info_map, (int64_t)c->cur_file_no);
            if (c->tree_info == NULL) {
                c->tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
                c->tree_info->physical_init_pos = *c->last_physical_pos;
                ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);
            } else {
                if (c->tree_info->physical_size > 200 * 1024 && c->tree_info->file_size > 0) {
                    c->logical_size = c->tree_info->file_size;
                    c->async_open = 1;
                } else {
                    c->async_open = 0;
                }
            }
        } while(0);
    }

    ret = ijkio_alloc_url(&(c->inner), url);
    if (c->inner && !ret) {
        c->inner->ijkio_app_ctx = c->ijkio_app_ctx;
        if (c->logical_size <= 0 || c->async_open == 0) {
            c->async_open = 0;
            ret = ijkio_cache_io_open(h, url, flags, options);
            if (ret != 0)
                goto url_fail;
        } else {
            c->tree_info->file_size = c->logical_size;
            ijk_av_dict_copy(&c->inner_options, *options, 0);
            strcpy(c->inner_url, url);
            c->inner_flags = flags;
            call_inject_statistic(h);
        }
    }

    ret = pthread_mutex_init(&c->file_mutex, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "pthread_mutex_init failed : %s\n", av_err2str(ret));
        goto file_mutex_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_main, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_main_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_file_background, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_file_background_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_exit, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_exit_fail;
    }

    if (!c->cache_file_close && c->cache_file_forwards_capacity) {
        c->task_is_running = 1;
        ret = ijk_threadpool_add(c->threadpool_ctx, ijkio_cache_task, h, NULL, 0);
        if (ret) {
            c->task_is_running = 0;
            pthread_cond_signal(&c->cond_wakeup_exit);
            goto thread_fail;
        }
    }

    return 0;

thread_fail:
    pthread_cond_destroy(&c->cond_wakeup_exit);
cond_wakeup_exit_fail:
    pthread_cond_destroy(&c->cond_wakeup_file_background);
cond_wakeup_file_background_fail:
    pthread_cond_destroy(&c->cond_wakeup_main);
cond_wakeup_main_fail:
    pthread_mutex_destroy(&c->file_mutex);
file_mutex_fail:
    if (c->async_open) {
        if (c->inner_options) {
            ijk_av_dict_free(&c->inner_options);
        }
    } else {
        if (c->inner) {
            if (c->inner->prot && c->inner->prot->url_close) {
                c->inner->prot->url_close(c->inner);
            }
        }
    }
url_fail:
    if (c->inner) {
        ijk_av_freep(&c->inner->priv_data);
        ijk_av_freep(&c->inner);
    }
    return ret;
}

static int ijkio_file_read(IjkURLContext *h, void *dest, int to_read)
{
    IjkIOCacheContext *c   = h->priv_data;
    IjkCacheEntry *entry   = NULL;
    IjkCacheEntry *next[2] = {NULL, NULL};
    int64_t ret            = 0;
    int to_copy            = 0;

    if (!c->tree_info)
        return 0;

    entry = ijk_av_tree_find(c->tree_info->root, &c->read_logical_pos, cmp, (void**)next);
    if (!entry)
        entry = next[0];

    if (entry) {
        int64_t in_block_pos = c->read_logical_pos - entry->logical_pos;
        if (in_block_pos < entry->size && entry->logical_pos <= c->read_logical_pos) {
            int64_t physical_target = entry->physical_pos + in_block_pos;
            if (c->cache_physical_pos != physical_target) {
                ret = lseek(c->fd, physical_target, SEEK_SET);
                if (ret < 0) {
                    c->file_handle_retry_count++;
                    ijkio_cache_file_error(h);
                }
            } else {
                ret = c->cache_physical_pos;
            }

            if (ret >= 0) {
                to_copy = (int)FFMIN(to_read, entry->size - in_block_pos);
                ret = wrapped_file_read(h, dest, to_copy);
                if (ret < 0) {
                    if(c->read_file_inner_error) {
                        c->file_handle_retry_count++;
                        ijkio_cache_file_error(h);
                    }
                }
            }
        }
    }
    return (int)ret;
}

static int64_t sync_add_entry(IjkURLContext *h, const unsigned char *buf, int size)
{
    IjkIOCacheContext *c= h->priv_data;
    int64_t pos = -1;
    int64_t ret = 0;
    IjkCacheEntry *entry = NULL, *next[2] = {NULL, NULL};
    IjkCacheEntry *entry_ret = NULL;
    struct IjkAVTreeNode *node = NULL;
    int64_t free_space = 0;

    if (*c->last_physical_pos != c->cache_physical_pos) {
        pos = lseek(c->fd, *c->last_physical_pos, SEEK_SET);
        if (pos < 0) {
            return FILE_RW_ERROR;
        } else {
            c->cache_physical_pos = pos;
            *c->last_physical_pos = pos;
        }
    } else {
        pos = *c->last_physical_pos;
    }

    if (*c->last_physical_pos + size >= c->cache_max_capacity) {
        free_space = ijkio_cache_file_overrang(h, &pos, size);
        if (free_space < size || pos < 0) {
            return FILE_RW_ERROR;
        }
        c->cache_physical_pos = pos;
        *c->last_physical_pos = pos;
    }

    ret = write(c->fd, buf, size);
    if (ret < 0) {
        return FILE_RW_ERROR;
    }

    c->cache_physical_pos       += ret;
    *c->last_physical_pos       += ret;
    c->tree_info->physical_size += ret;

    entry = ijk_av_tree_find(c->tree_info->root, &c->read_logical_pos, cmp, (void**)next);

    if (!entry)
        entry = next[0];

    if (!entry ||
        entry->logical_pos  + entry->size != c->read_logical_pos ||
        entry->physical_pos + entry->size != pos) {
        entry = malloc(sizeof(*entry));
        node = ijk_av_tree_node_alloc();
        if (!entry || !node) {
            ret = IJKAVERROR(ENOMEM);
            goto fail;
        }
        entry->logical_pos = c->read_logical_pos;
        entry->physical_pos = pos;
        entry->size = ret;

        entry_ret = ijk_av_tree_insert(&c->tree_info->root, entry, cmp, &node);
        if (entry_ret && entry_ret != entry) {
            ret = -1;
            av_log(NULL, AV_LOG_ERROR, "sync_add_entry av_tree_insert failed\n");
            goto fail;
        }
    } else {
        entry->size += ret;
    }

    return ret;
fail:
    //we could truncate the file to pos here if pos >=0 but ftruncate isn't available in VS so
    //for simplicty we just leave the file a bit larger
    free(entry);
    free(node);
    return ret;
}

static int ijkio_cache_sync_read(IjkURLContext *h, unsigned char *buf, int size) {
    IjkIOCacheContext *c= h->priv_data;
    int64_t ret = 0;
    int to_read = size;
    int to_copy = 0;
    IjkCacheEntry *entry = NULL, *next_entry = NULL, *next[2] = {NULL, NULL};

    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (to_read <= 0) {
        return to_read;
    }

    if (c->tree_info) {
        entry = ijk_av_tree_find(c->tree_info->root, &c->read_logical_pos, cmp, (void**)next);
    }

    if (!entry)
        entry = next[0];

    if (entry) {
        int64_t in_block_pos = c->read_logical_pos - entry->logical_pos;
        if (in_block_pos < entry->size && entry->logical_pos <= c->read_logical_pos) {
            int64_t physical_target = entry->physical_pos + in_block_pos;
            if (c->cache_physical_pos != physical_target) {
                ret = lseek(c->fd, physical_target, SEEK_SET);
            } else {
                ret = c->cache_physical_pos;
            }

            if (ret >= 0) {
                c->cache_physical_pos = ret;
                to_copy = (int)FFMIN(to_read, entry->size - in_block_pos);
                ret = wrapped_file_read(h, buf, to_copy);
                if (ret >= 0) {
                    c->cache_physical_pos += ret;
                    return (int)ret;
                }
            }

            av_log(NULL, AV_LOG_ERROR, "%s cache file is bad, will try recreate\n", __func__);
            ijk_map_traversal_handle(c->cache_info_map, NULL, tree_destroy);
            ijk_map_clear(c->cache_info_map);
            c->tree_info             = NULL;
            *c->last_physical_pos    = 0;
            c->cache_physical_pos    = 0;
            c->io_eof_reached        = 0;
            close(c->fd);
            c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0600);
            c->ijkio_app_ctx->fd = c->fd;
            if (c->fd >= 0) {
                c->tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
                if (c->tree_info) {
                    ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);
                }
            }
        }
    }

    if (c->read_logical_pos >= c->logical_size) {
        c->io_eof_reached = 1;
        return 0;
    }

    if (c->async_open > 0) {
        ret = ijkio_cache_io_open(h, c->inner_url, c->inner_flags, &c->inner_options);
        if (ret != 0) {
            return (int)ret;
        }
        c->async_open = 0;
    }

    if (c->read_inner_pos != c->read_logical_pos) {
        ret = c->inner->prot->url_seek(c->inner, c->read_logical_pos, SEEK_SET);
        if (ret < 0) {
            return (int)ret;
        }
        c->read_inner_pos = ret;
    }

    next_entry = next[1];
    if (next_entry && next_entry->logical_pos > c->read_logical_pos) {
        to_copy = (int)FFMIN(to_read, next_entry->logical_pos - c->read_logical_pos);
    } else {
        to_copy = to_read;
    }

    ret = wrapped_url_read(h, buf, to_copy);
    if (ret <= 0)
        return (int)ret;

    c->read_inner_pos   += ret;

    if (c->fd >= 0 && c->tree_info && !c->only_read_file) {
        sync_add_entry(h, buf, (int)ret);
    }

    return (int)ret;
}

static int ijkio_cache_read(IjkURLContext *h, unsigned char *buf, int size) {
    IjkIOCacheContext *c = h->priv_data;
    int64_t          ret = 0;
    int          to_read = size;
    unsigned char  *dest = buf;
    int          to_copy = 0;

    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (c->cache_file_close) {
        return wrapped_url_read(h, dest, to_read);
    }

    if (!c->cache_file_forwards_capacity) {
        ret = ijkio_cache_sync_read(h, buf, size);
        if (ret >= 0) {
            c->read_logical_pos += ret;
        }
        call_inject_statistic(h);
        return (int)ret;
    }

    pthread_mutex_lock(&c->file_mutex);
    while (to_read > 0) {
        if (ijkio_cache_check_interrupt(h)) {
            ret = IJKAVERROR_EXIT;
            break;
        }

        if (c->cache_file_close) {
            ret = c->inner->prot->url_seek(c->inner, c->read_logical_pos, SEEK_SET);
            if (ret < 0) {
                pthread_mutex_unlock(&c->file_mutex);
                return (int)ret;
            }

            to_copy  = wrapped_url_read(h, dest, to_read);
            to_read -= to_copy;
            ret      = size - to_read;
            pthread_mutex_unlock(&c->file_mutex);
            return (int)ret;
        }

        to_copy = ijkio_file_read(h, dest, to_read);
        if (to_copy > 0) {
            to_read             -= to_copy;
            ret                  = size - to_read;
            dest                += to_copy;
            c->read_logical_pos += to_copy;
            if (to_read <= 0)
                break;
        } else if (c->io_eof_reached) {
            if (ret <= 0) {
                if (c->io_error)
                    ret = c->io_error;
                else
                    ret = IJKAVERROR_EOF;
            }
            break;
        }
        pthread_cond_signal(&c->cond_wakeup_file_background);
        pthread_cond_wait(&c->cond_wakeup_main, &c->file_mutex);
    }

    if (ret != size || (!c->io_eof_reached && (c->file_logical_pos - c->read_logical_pos) <= c->cache_file_forwards_capacity)) {
        pthread_cond_signal(&c->cond_wakeup_file_background);
    }
    pthread_mutex_unlock(&c->file_mutex);
    return (int)ret;
}

static int64_t ijkio_cache_seek(IjkURLContext *h, int64_t pos, int whence) {
    IjkIOCacheContext *c= h->priv_data;
    int64_t ret = 0;
    int64_t new_logical_pos = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (whence == IJKAVSEEK_SIZE) {
        return c->logical_size;
    } else if (whence == SEEK_CUR) {
        new_logical_pos = pos + c->read_logical_pos;
    } else if (whence == SEEK_SET){
        new_logical_pos = pos;
    } else {
        return IJKAVERROR(EINVAL);
    }
    if (new_logical_pos < 0)
        return IJKAVERROR(EINVAL);

    if (c->cache_file_close) {
        return c->inner->prot->url_seek(c->inner, new_logical_pos, SEEK_SET);
    }

    if (!c->cache_file_forwards_capacity) {
        c->read_logical_pos = new_logical_pos;
        return new_logical_pos;
    }

    pthread_mutex_lock(&c->file_mutex);
    c->seek_request   = 1;
    c->seek_pos       = new_logical_pos;
    c->seek_whence    = SEEK_SET;
    c->seek_completed = 0;

    while (1) {
        if (ijkio_cache_check_interrupt(h)) {
            ret = IJKAVERROR_EXIT;
            break;
        }
        if (c->seek_completed) {
            ret = c->seek_ret;
            break;
        }
        pthread_cond_signal(&c->cond_wakeup_file_background);
        pthread_cond_wait(&c->cond_wakeup_main, &c->file_mutex);
    }

    pthread_mutex_unlock(&c->file_mutex);
    return ret;
}

static int ijkio_cache_close(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int              ret = 0;

    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (c->cache_file_forwards_capacity) {
        pthread_mutex_lock(&c->file_mutex);
        c->abort_request = 1;
        pthread_cond_signal(&c->cond_wakeup_file_background);
        while (c->task_is_running) {
            pthread_cond_wait(&c->cond_wakeup_exit, &c->file_mutex);
        }
        pthread_mutex_unlock(&c->file_mutex);
    } else {
        c->abort_request = 1;
    }

    pthread_cond_destroy(&c->cond_wakeup_file_background);
    pthread_cond_destroy(&c->cond_wakeup_main);
    pthread_cond_destroy(&c->cond_wakeup_exit);
    pthread_mutex_destroy(&c->file_mutex);

    ret = c->inner->prot->url_close(c->inner);

    if (c->inner_options) {
        ijk_av_dict_free(&c->inner_options);
    }
    ijk_av_freep(&c->inner->priv_data);

    ijk_av_freep(&c->inner);
    return ret;
}

static int ijkio_cache_pause(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (c->inner->prot->url_pause) {
        ret = c->inner->prot->url_pause(c->inner);
    }

    if (!c->cache_file_forwards_capacity) {
        c->abort_request = 1;
    } else {
        pthread_mutex_lock(&c->file_mutex);
        c->abort_request = 1;
        pthread_cond_signal(&c->cond_wakeup_file_background);
        while (c->task_is_running) {
            pthread_cond_wait(&c->cond_wakeup_exit, &c->file_mutex);
        }
        pthread_mutex_unlock(&c->file_mutex);
    }

    return ret;
}

static int ijkio_cache_resume(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (!c->cache_file_path || 0 == strlen(c->cache_file_path) || c->cache_file_close) {
        c->cache_file_close = 1;
    } else {
        if (c->cache_file_forwards_capacity) {
            int64_t seek_ret = lseek(c->fd, *c->last_physical_pos, SEEK_SET);
            if (seek_ret < 0) {
                c->cache_file_close = 1;
                close(c->fd);
                c->fd = -1;
                c->ijkio_app_ctx->fd = -1;
            } else {
                c->cache_physical_pos = *c->last_physical_pos;
            }
        }
    }

    if (c->inner->prot->url_resume) {
        ret = c->inner->prot->url_resume(c->inner);
        if (ret != 0) {
            return ret;
        }
    }

    c->abort_request = 0;

    if (!c->cache_file_close && c->cache_file_forwards_capacity) {
        c->task_is_running = 1;
        ret = ijk_threadpool_add(c->threadpool_ctx, ijkio_cache_task, h, NULL, 0);
        if (ret) {
            c->task_is_running = 0;
            pthread_cond_signal(&c->cond_wakeup_exit);
        }
    }
    return ret;
}

IjkURLProtocol ijkio_cache_protocol = {
    .name                = "ijkiocache",
    .url_open2           = ijkio_cache_open,
    .url_read            = ijkio_cache_read,
    .url_seek            = ijkio_cache_seek,
    .url_close           = ijkio_cache_close,
    .url_pause           = ijkio_cache_pause,
    .url_resume          = ijkio_cache_resume,
    .priv_data_size      = sizeof(IjkIOCacheContext),
};
