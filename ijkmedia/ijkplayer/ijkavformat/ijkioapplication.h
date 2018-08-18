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

#ifndef IJKAVFORMAT_IJKIOAPPLICATION_H
#define IJKAVFORMAT_IJKIOAPPLICATION_H

#include "ijkplayer/ijkavutil/ijkutils.h"
#include "ijkplayer/ijkavutil/ijkthreadpool.h"

#include <stdint.h>

#define CACHE_FILE_PATH_MAX_LEN        512
#define IJKIOAPP_EVENT_CACHE_STATISTIC 0x1003  //IJKIOAppCacheStatistic share with avutil/application.h

typedef struct IjkIOAppCacheStatistic {
    int64_t cache_physical_pos;
    int64_t cache_file_forwards;
    int64_t cache_file_pos;
    int64_t cache_count_bytes;
    int64_t logical_file_size;
} IjkIOAppCacheStatistic;

typedef struct IjkCacheEntry {
    int64_t logical_pos;
    int64_t physical_pos;
    int64_t size;
} IjkCacheEntry;

typedef struct IjkIOApplicationContext IjkIOApplicationContext;
struct IjkIOApplicationContext {
    IjkThreadPoolContext *threadpool_ctx;
    IjkAVIOInterruptCB *ijkio_interrupt_callback;
    char cache_file_path[CACHE_FILE_PATH_MAX_LEN];
    int64_t last_physical_pos;
    void *cache_info_map;
    void *opaque;
    int64_t cache_count_bytes;
    int fd;
    pthread_mutex_t mutex;
    int shared;
    int active_reconnect;
    int (*func_ijkio_on_app_event)(IjkIOApplicationContext *h, int event_type ,void *obj, int size);
};

int  ijkio_application_alloc(IjkIOApplicationContext **ph, void *opaque);
int  ijkio_application_open(IjkIOApplicationContext **ph, void *opaque);
void ijkio_application_close(IjkIOApplicationContext *h);
void ijkio_application_closep(IjkIOApplicationContext **ph);

void ijkio_application_on_cache_statistic(IjkIOApplicationContext *h, IjkIOAppCacheStatistic *statistic);

#endif /* IJKAVFORMAT_IJKIOAPPLICATION_H */
