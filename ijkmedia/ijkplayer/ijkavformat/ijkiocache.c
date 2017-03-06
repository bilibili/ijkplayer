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
#include "ijkplayer/ijkavutil/ijkfifo.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#define DEFAULT_CACHE_MAX_CAPACITY            (521 * 1024 * 1024)
#define DEFAULT_CACHE_FORWARDS_FIFO_CAPACITY  (8 * 1024 * 1024)
#define DEFAULT_CACHE_FILE_FORWARDS_CAPACITY  (8 * 1024 * 1024)
#   ifndef O_BINARY
#       define O_BINARY 0
#   endif

typedef struct IjkIOCacheContext {
    char *cache_file_path;
    int fd;
    IjkCacheTreeInfo *tree_info;
    int64_t logical_size;
    int64_t read_logical_pos;
    int64_t file_logical_pos;
    int64_t fifo_logical_pos;
    int64_t cache_physical_pos;
    int64_t file_inner_pos;
    int64_t file_logical_end;
    int64_t cache_max_capacity;
    int64_t cache_forwards_fifo_capacity;
    int64_t cache_file_forwards_capacity;
    int cache_file_close;
    int fifo_pos_reset;
    int io_eof_reached;
    int fifo_eof_reached;
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
    int64_t *cache_limit_file_pos;
    int64_t *cache_count_bytes;

    IjkFifoBuffer      *fifo;
    int64_t            physical_fifo_pos;
    pthread_cond_t     cond_wakeup_main;
    pthread_cond_t     cond_wakeup_background;
    pthread_cond_t     cond_wakeup_exit;
    pthread_mutex_t    mutex;
    int                abort_request;
    IjkAVIOInterruptCB *ijkio_interrupt_callback;
    int task_is_running;

    IjkURLContext *inner;
    IjkThreadPoolContext *threadpool_ctx;
    IjkIOApplicationContext *ijkio_app_ctx;
} IjkIOCacheContext;

typedef struct IjkCacheEntry {
    int64_t logical_pos;
    int64_t physical_pos;
    int64_t size;
} IjkCacheEntry;

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
        statistic.cache_buf_forwards  = ijk_av_fifo_size(c->fifo);
        statistic.cache_file_pos      = c->file_logical_pos;
        statistic.cache_count_bytes         = *c->cache_count_bytes;
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

static int tree_destroy(void *elem)
{
    IjkCacheTreeInfo *info = elem;
    ijk_av_tree_enumerate(info->root, NULL, NULL, enu_free);
    ijk_av_tree_destroy(info->root);
    free(info);
    return 0;
}

static int ijkio_cache_file_error(IjkURLContext *h) {
    IjkIOCacheContext *c= h->priv_data;
    if (c && c->file_handle_retry_count > 3) {
        c->file_error_count++;
        ijk_map_traversal_handle(c->cache_info_map, tree_destroy);
        ijk_map_clear(c->cache_info_map);
        c->tree_info = NULL;
        *c->last_physical_pos    = 0;
        c->cache_physical_pos    = 0;
        c->file_inner_pos        = 0;
        c->io_eof_reached        = 0;
        c->file_logical_pos      = c->fifo_logical_pos;
        close(c->fd);
        c->fd = -1;
        if (c->file_error_count > 3) {
            c->fifo_pos_reset = 1;
            c->cache_file_close = 1;
            close(c->fd);
            remove(c->cache_file_path);
            c->fd = -1;
            return 0;
        }
        c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0600);
        if (c->fd >= 0) {
            c->file_handle_retry_count = 0;
            c->tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
        } else {
            c->fifo_pos_reset = 1;
            c->cache_file_close = 1;
        }
    }

    return 0;
}

static int64_t ijkio_cache_file_overrang(IjkURLContext *h, int64_t *cur_pos, int size) {
    IjkIOCacheContext *c= h->priv_data;
    IjkCacheTreeInfo *tree_info = NULL;
    int64_t free_space   = 0;
    int64_t min_root_id      = 0;
    int is_first_pos     = 1;

    if (1 == ijk_map_size(c->cache_info_map)) {
        tree_info = ijk_map_index_get(c->cache_info_map, 0);
        free_space += tree_info->physical_size;
        *cur_pos = lseek(c->fd, 0, SEEK_SET);
        *c->cache_limit_file_pos = c->cache_max_capacity;
        ijk_av_tree_enumerate(tree_info->root, NULL, NULL, enu_free);
        ijk_av_tree_destroy(tree_info->root);
        memset(c->tree_info, 0, sizeof(IjkCacheTreeInfo));
        c->file_logical_pos   = c->fifo_logical_pos;
        c->io_eof_reached     = 0;
        *c->last_physical_pos = 0;
        c->cache_physical_pos = 0;
        return c->cache_max_capacity;
    } else {
        ijk_map_remove(c->cache_info_map, (int64_t)c->cur_file_no);
    }

    while (1) {
        min_root_id = ijk_map_get_min_key(c->cache_info_map);
        if (min_root_id >= 0)
            tree_info = ijk_map_get(c->cache_info_map, min_root_id);
        else
            break;

        if (tree_info) {
            free_space += tree_info->physical_size;
            int64_t pos = lseek(c->fd, tree_info->physical_init_pos, SEEK_SET);
            if (pos < 0) {
                return -1;
            }
            if (is_first_pos) {
                *cur_pos = pos;
                is_first_pos = 0;
            }
            ijk_map_remove(c->cache_info_map, min_root_id);
            *c->last_physical_pos = pos;
            c->cache_physical_pos = pos;
            *c->cache_limit_file_pos = tree_info->physical_size + tree_info->physical_init_pos;

            if (*c->cache_limit_file_pos > c->cache_max_capacity)
                *c->cache_limit_file_pos = c->cache_max_capacity;

            ijk_av_tree_enumerate(tree_info->root, NULL, NULL, enu_free);
            ijk_av_tree_destroy(tree_info->root);
            free(tree_info);
        } else {
            break;
        }

        if (free_space >= size) {
            break;
        }
    }

    ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);

    return free_space;
}

static int64_t add_entry(IjkURLContext *h, const unsigned char *buf, int size)
{
    IjkIOCacheContext *c= h->priv_data;
    int64_t pos = -1;
    int64_t ret;
    IjkCacheEntry *entry = NULL, *next[2] = {NULL, NULL};
    IjkCacheEntry *entry_ret;
    struct IjkAVTreeNode *node = NULL;
    int64_t free_space;

    //FIXME avoid lseek
    pos = lseek(c->fd, *c->last_physical_pos, SEEK_SET);

    if (pos < 0) {
        ret = IJKAVERROR(errno);
        c->file_handle_retry_count++;
        ijkio_cache_file_error(h);
        return ret;
    } else {
        c->cache_physical_pos = pos;
        *c->last_physical_pos = pos;
    }

    if (pos + size >= *c->cache_limit_file_pos) {
        free_space = ijkio_cache_file_overrang(h, &pos, size);
        if (free_space < size) {
            c->fifo_pos_reset = 1;
            c->cache_file_close = 1;
            return -1;
        }
        if (pos < 0) {
            ret = IJKAVERROR(errno);
            c->file_handle_retry_count++;
            ijkio_cache_file_error(h);
            return ret;
        }
        if (free_space == c->cache_max_capacity)
            return 0;
    }

    ret = write(c->fd, buf, size);
    if (ret < 0) {
        ret = IJKAVERROR(errno);
        c->file_handle_retry_count++;
        ijkio_cache_file_error(h);
        return ret;
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
        entry->physical_pos + entry->size != pos
    ) {
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
            // av_log(h, AV_LOG_ERROR, "av_tree_insert failed\n");
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

static int wrapped_file_read(void *src, void *dst, int size)
{
    IjkURLContext     *h   = src;
    IjkIOCacheContext *c   = h->priv_data;
    int ret;

    ret = (int)read(c->fd, dst, size);
    c->read_file_inner_error = ret < 0 ? ret : 0;
    return ret;
}

static int wrapped_url_read(void *src, void *dst, int size)
{
    IjkURLContext     *h   = src;
    IjkIOCacheContext *c   = h->priv_data;
    int ret;

    ret = c->inner->prot->url_read(c->inner, dst, size);

    if (ret > 0)
        *c->cache_count_bytes += ret;

    c->inner_io_error = ret < 0 ? ret : 0;

    return ret;
}

static int64_t ijkio_cache_fill_fifo(IjkURLContext *h) {
    IjkIOCacheContext *c= h->priv_data;
    if (!c)
        return IJKAVERROR(ENOSYS);

    int to_copy, fifo_space;
    IjkCacheEntry *entry, *next[2] = {NULL, NULL};
    int64_t r = 0;
    pthread_mutex_lock(&c->mutex);
    if (ijkio_cache_check_interrupt(h)) {
        c->io_eof_reached   = 1;
        c->fifo_eof_reached = 1;
        c->io_error         = IJKAVERROR_EXIT;
        pthread_cond_signal(&c->cond_wakeup_main);
        pthread_mutex_unlock(&c->mutex);
        return IJKAVERROR_EXIT;
    }

    if (c->seek_request) {
        c->io_eof_reached   = 0;
        c->fifo_eof_reached = 0;
        c->io_error         = 0;

        c->seek_completed   = 1;
        c->seek_request     = 0;
        c->fifo_logical_pos = c->seek_pos;
        c->file_logical_pos = c->seek_pos;
        c->seek_ret         = c->seek_pos;

        ijk_av_fifo_reset(c->fifo);
        if (c->cache_file_close) {
            if (!c->inner || !c->inner->prot || !c->inner->prot) {
                c->seek_ret = IJKAVERROR(ENOSYS);
            } else {
                c->seek_ret = c->inner->prot->url_seek(c->inner, c->fifo_logical_pos, SEEK_SET);
            }
        }
    }

    fifo_space = ijk_av_fifo_space(c->fifo);

    if (c->cache_file_close) {
        if (c->fifo_eof_reached || fifo_space <= 0) {
            pthread_cond_signal(&c->cond_wakeup_main);
            pthread_cond_wait(&c->cond_wakeup_background, &c->mutex);
            pthread_mutex_unlock(&c->mutex);
            return 0;
        }
    } else {
        int64_t file_forwards_spece = c->file_logical_pos - c->fifo_logical_pos;
        if (c->fifo_eof_reached || fifo_space <= 0) {
            pthread_cond_signal(&c->cond_wakeup_main);
            if ((file_forwards_spece >= c->cache_file_forwards_capacity) || c->io_eof_reached) {
                pthread_cond_wait(&c->cond_wakeup_background, &c->mutex);
            }
            pthread_mutex_unlock(&c->mutex);
            return 0;
        }
    }

    pthread_mutex_unlock(&c->mutex);
    to_copy = FFMIN(4096, fifo_space);

    if (to_copy <= 0) {
        pthread_cond_signal(&c->cond_wakeup_main);
        return r;
    }

    if (!c->cache_file_close) {
        entry = ijk_av_tree_find(c->tree_info->root, &c->fifo_logical_pos, cmp, (void**)next);
        if (!entry)
            entry = next[0];

        if (entry) {
            int64_t in_block_pos = c->fifo_logical_pos - entry->logical_pos;
            assert(entry->logical_pos <= c->fifo_logical_pos);
            if (in_block_pos < entry->size) {
                int64_t physical_target = entry->physical_pos + in_block_pos;
                if (c->cache_physical_pos != physical_target) {
                    r = lseek(c->fd, physical_target, SEEK_SET);
                    if (r < 0) {
                        c->file_handle_retry_count++;
                        ijkio_cache_file_error(h);
                    }
                } else {
                    r = c->cache_physical_pos;
                }

                if (r >= 0) {
                    to_copy = (int)FFMIN(to_copy, entry->size - in_block_pos);
                    r = ijk_av_fifo_generic_write(c->fifo, h, to_copy, wrapped_file_read);
                    if (r > 0) {
                        c->fifo_logical_pos       += r;
                    } else {
                        if(c->read_file_inner_error) {
                            c->file_handle_retry_count++;
                            ijkio_cache_file_error(h);
                        }
                    }
                }
            } else {
                if (c->io_eof_reached)
                    c->fifo_eof_reached = 1;
            }
        } else {
            if (c->io_eof_reached)
                c->fifo_eof_reached = 1;
        }
    } else {
        if (!c->inner || !c->inner->prot || !c->inner->prot) {
            r = IJKAVERROR(ENOSYS);
        } else {
            if (c->fifo_pos_reset) {
                r = c->inner->prot->url_seek(c->inner, c->fifo_logical_pos, SEEK_SET);
                if (r < 0)
                    return r;

                c->fifo_pos_reset = 0;
            }

            r = ijk_av_fifo_generic_write(c->fifo, h, to_copy, wrapped_url_read);
            if (r > 0) {
                c->fifo_logical_pos += r;
            } else {
                c->io_eof_reached = 1;
                c->fifo_eof_reached = 1;
                if (c->inner_io_error < 0)
                    c->io_error = c->inner_io_error;
            }
        }
    }

    pthread_cond_signal(&c->cond_wakeup_main);
    return r;
}

static int64_t ijkio_cache_write_file(IjkURLContext *h) {
    IjkIOCacheContext *c= h->priv_data;
    int64_t r;
    unsigned char buf[4096] = {0};
    int to_read = 4096;
    int64_t to_copy = (int64_t)to_read;

    IjkCacheEntry *root = NULL ,*l_entry = NULL, *r_entry = NULL, *next[2] = {NULL, NULL};

    if (!c || !c->inner || !c->inner->prot || !c->inner->prot)
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

    r = add_entry(h, buf, (int)r);

    if (r > 0)
        c->file_logical_pos += r;

    return r;
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

static void ijkio_cache_task(void *h, void *r) {
    int64_t ret;
    IjkIOCacheContext *c= ((IjkURLContext *)h)->priv_data;
    c->task_is_running = 1;
    while(1) {
        ret = ijkio_cache_fill_fifo(h);
        if (ret == IJKAVERROR_EXIT)
            break;

        if (!c->cache_file_close &&
            (c->file_logical_pos - c->fifo_logical_pos <= c->cache_file_forwards_capacity)
            && !c->io_eof_reached) {
            ijkio_cache_write_file(h);
        }

        call_inject_statistic(h);
    }
    pthread_mutex_lock(&c->mutex);
    c->task_is_running = 0;
    pthread_cond_signal(&c->cond_wakeup_exit);
    pthread_mutex_unlock(&c->mutex);
}

static int ijkio_cache_open(IjkURLContext *h, const char *url, int flags, IjkAVDictionary **options) {
    IjkIOCacheContext *c= h->priv_data;
    int ret;
    if (!c)
        return IJKAVERROR(ENOSYS);

    c->ijkio_app_ctx = h->ijkio_app_ctx;
    if (c->ijkio_app_ctx == NULL) {
        return -1;
    }

    c->ijkio_interrupt_callback = h->ijkio_app_ctx->ijkio_interrupt_callback;

    ijk_av_strstart(url, "cache:", &url);
    c->cache_forwards_fifo_capacity = DEFAULT_CACHE_FORWARDS_FIFO_CAPACITY;
    c->cache_file_forwards_capacity = DEFAULT_CACHE_FILE_FORWARDS_CAPACITY;
    c->cache_max_capacity = DEFAULT_CACHE_MAX_CAPACITY;

    IjkAVDictionaryEntry *t = NULL;
    t = ijk_av_dict_get(*options, "cache_max_capacity", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        c->cache_max_capacity = strtoll(t->value, NULL, 10);
    }
    t = ijk_av_dict_get(*options, "cache_forwards_fifo_capacity", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        c->cache_forwards_fifo_capacity = strtoll(t->value, NULL, 10);
    }
    t = ijk_av_dict_get(*options, "cache_file_forwards_capacity", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        c->cache_file_forwards_capacity = strtoll(t->value, NULL, 10);
    }

    t = ijk_av_dict_get(*options, "cache_file_close", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        c->cache_file_close = (int)strtol(t->value, NULL, 10);
        c->cache_file_close = c->cache_file_close != 0 ? 1 : 0;
    }

    t = ijk_av_dict_get(*options, "cur_file_no", NULL, IJK_AV_DICT_IGNORE_SUFFIX);
    if (t) {
        c->cur_file_no = (int)strtol(t->value, NULL, 10);
    }

    c->cache_file_path = c->ijkio_app_ctx->cache_file_path;

    if (c->cache_file_path == NULL || 0 == strlen(c->cache_file_path)) {
        c->cache_file_close = 1;
    }

    c->threadpool_ctx       = c->ijkio_app_ctx->threadpool_ctx;
    c->cache_info_map       = c->ijkio_app_ctx->cache_info_map;
    c->last_physical_pos    = &c->ijkio_app_ctx->last_physical_pos;
    c->cache_limit_file_pos = &c->ijkio_app_ctx->cache_limit_file_pos;
    c->cache_count_bytes    = &c->ijkio_app_ctx->cache_count_bytes;
    if (!c->last_physical_pos || !c->threadpool_ctx || !c->cache_info_map) {
        return -1;
    }

    if (!c->cache_file_close) {
        do {
            c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT, 0600);
            if (c->fd < 0) {
                c->cache_file_close = 1;
                break;
            }

            int64_t seek_ret = lseek(c->fd, *c->last_physical_pos, SEEK_SET);
            if (seek_ret < 0) {
                c->cache_file_close = 1;
                close(c->fd);
                break;
            } else {
                c->cache_physical_pos = *c->last_physical_pos;
            }

            c->tree_info = ijk_map_get(c->cache_info_map, (int64_t)c->cur_file_no);
            if (c->tree_info == NULL) {
                c->tree_info = calloc(1, sizeof(IjkCacheTreeInfo));
                c->tree_info->physical_init_pos = *c->last_physical_pos;
            }

            ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);

            if (*c->cache_limit_file_pos <= 0)
                *c->cache_limit_file_pos = c->cache_max_capacity;
        } while(0);
    }

    c->fifo = ijk_av_fifo_alloc((unsigned int)c->cache_forwards_fifo_capacity);

    ret = ijkio_alloc_url(&(c->inner), url);
    if (c->inner && !ret) {
        c->inner->ijkio_app_ctx = c->ijkio_app_ctx;
        ret = c->inner->prot->url_open2(c->inner, url, flags, options);
        if (ret != 0)
            goto url_fail;
        else
            c->logical_size = ijkio_cache_ffurl_size(h);
    }

    ret = pthread_mutex_init(&c->mutex, NULL);
    if (ret != 0) {
        // av_log(h, AV_LOG_ERROR, "pthread_mutex_init failed : %s\n", av_err2str(ret));
        goto mutex_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_main, NULL);
    if (ret != 0) {
        // av_log(h, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_main_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_background, NULL);
    if (ret != 0) {
        // av_log(h, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_background_fail;
    }

    ret = pthread_cond_init(&c->cond_wakeup_exit, NULL);
    if (ret != 0) {
        // av_log(h, AV_LOG_ERROR, "pthread_cond_init failed : %s\n", av_err2str(ret));
        goto cond_wakeup_exit_fail;
    }

    c->task_is_running = 1;
    ret = ijk_threadpool_add(c->threadpool_ctx, ijkio_cache_task, h, NULL, 0);
    if (ret) {
        c->task_is_running = 0;
        pthread_cond_signal(&c->cond_wakeup_exit);
        goto thread_fail;
    }

    return 0;
thread_fail:
    pthread_cond_destroy(&c->cond_wakeup_exit);
cond_wakeup_exit_fail:
    pthread_cond_destroy(&c->cond_wakeup_background);
cond_wakeup_background_fail:
    pthread_cond_destroy(&c->cond_wakeup_main);
cond_wakeup_main_fail:
    pthread_mutex_destroy(&c->mutex);
mutex_fail:
    if (c->inner) {
        if (c->inner->prot && c->inner->prot->url_close)
            ret = c->inner->prot->url_close(c->inner);
        ijk_av_freep(&c->inner->priv_data);
        ijk_av_freep(&c->inner);
    }
url_fail:
    ijk_av_fifo_freep(&c->fifo);
    if (c->tree_info)
        ijk_map_put(c->cache_info_map, (int64_t)c->cur_file_no, c->tree_info);
    return ret;
}

static int ijkio_cache_read(IjkURLContext *h, unsigned char *buf, int size) {
    IjkIOCacheContext *c  = h->priv_data;
    int           to_read = size;
    int           ret     = 0;
    unsigned char *dst    = buf;
    if (!c)
        return IJKAVERROR(ENOSYS);
    pthread_mutex_lock(&c->mutex);
    while (to_read > 0) {
        int fifo_size, to_copy;
        if (ijkio_cache_check_interrupt(h)) {
            ret = IJKAVERROR_EXIT;
            break;
        }
        fifo_size = ijk_av_fifo_size(c->fifo);
        to_copy   = FFMIN(to_read, fifo_size);
        if (to_copy > 0) {
            ijk_av_fifo_generic_read(c->fifo, dst, to_copy, NULL);
            to_read             -= to_copy;
            ret                  = size - to_read;
            dst                 += to_copy;
            c->read_logical_pos += to_copy;
            if (to_read <= 0)
                break;
        } else if (c->fifo_eof_reached) {
            if (ret <= 0) {
                if (c->io_error)
                    ret = c->io_error;
                else
                    ret = IJKAVERROR_EOF;
            }
            break;
        }
        pthread_cond_signal(&c->cond_wakeup_background);
        pthread_cond_wait(&c->cond_wakeup_main, &c->mutex);
    }

    pthread_cond_signal(&c->cond_wakeup_background);
    pthread_mutex_unlock(&c->mutex);
    return ret;
}

static int64_t ijkio_cache_seek(IjkURLContext *h, int64_t pos, int whence) {
    IjkIOCacheContext *c= h->priv_data;
    int64_t ret, new_logical_pos;
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

    pthread_mutex_lock(&c->mutex);

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
            c->read_logical_pos = c->seek_pos;
            break;
        }
        pthread_cond_signal(&c->cond_wakeup_background);
        pthread_cond_wait(&c->cond_wakeup_main, &c->mutex);
    }

    pthread_mutex_unlock(&c->mutex);
    return ret;
}

static int ijkio_cache_close(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int      ret;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);
    pthread_mutex_lock(&c->mutex);
    c->abort_request = 1;
    pthread_cond_signal(&c->cond_wakeup_background);
    if (c->task_is_running) {
        pthread_cond_wait(&c->cond_wakeup_exit, &c->mutex);
    }
    pthread_mutex_unlock(&c->mutex);


    pthread_cond_destroy(&c->cond_wakeup_background);
    pthread_cond_destroy(&c->cond_wakeup_main);
    pthread_cond_destroy(&c->cond_wakeup_exit);
    pthread_mutex_destroy(&c->mutex);

    close(c->fd);

    ret = c->inner->prot->url_close(c->inner);
    ijk_av_fifo_freep(&c->fifo);

    ijk_av_freep(&c->inner->priv_data);

    ijk_av_freep(&c->inner);
    return ret;
}

static int ijkio_cache_pause(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    pthread_mutex_lock(&c->mutex);
    c->abort_request = 1;
    pthread_cond_signal(&c->cond_wakeup_background);
    if (c->task_is_running) {
        pthread_cond_wait(&c->cond_wakeup_exit, &c->mutex);
    }
    close(c->fd);
    c->fd = -1;
    pthread_mutex_unlock(&c->mutex);

    if (c->inner->prot->url_pause) {
        ret = c->inner->prot->url_pause(c->inner);
    }

    return ret;
}

static int ijkio_cache_resume(IjkURLContext *h) {
    IjkIOCacheContext *c = h->priv_data;
    int             ret  = 0;
    if (!c || !c->inner || !c->inner->prot)
        return IJKAVERROR(ENOSYS);

    if (c->cache_file_path == NULL || 0 == strlen(c->cache_file_path)) {
        c->cache_file_close = 1;
    } else {
        c->fd = open(c->cache_file_path, O_RDWR | O_BINARY | O_CREAT, 0600);
        if (c->fd < 0) {
            c->cache_file_close = 1;
        }

        int64_t seek_ret = lseek(c->fd, *c->last_physical_pos, SEEK_SET);
        if (seek_ret < 0) {
            c->cache_file_close = 1;
            close(c->fd);
        } else {
            c->cache_physical_pos = *c->last_physical_pos;
        }
    }

    if (c->inner->prot->url_resume) {
        ret = c->inner->prot->url_resume(c->inner);
        if (ret != 0) {
            return ret;
        }
    }

    c->abort_request = 0;

    c->task_is_running = 1;
    ret = ijk_threadpool_add(c->threadpool_ctx, ijkio_cache_task, h, NULL, 0);
    if (ret) {
        c->task_is_running = 0;
        pthread_cond_signal(&c->cond_wakeup_exit);
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
