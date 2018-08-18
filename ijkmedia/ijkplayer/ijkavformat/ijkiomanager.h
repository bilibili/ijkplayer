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

#ifndef IJKAVFORMAT_IJKIOMANAGER_H
#define IJKAVFORMAT_IJKIOMANAGER_H

#include "ijkiourl.h"
#include "ijkioapplication.h"

#include <stdint.h>

#define CACHE_MAP_PATH_MAX_LEN        512
typedef struct IjkIOManagerContext IjkIOManagerContext;
struct IjkIOManagerContext {
    IjkAVIOInterruptCB *ijkio_interrupt_callback;
    IjkIOApplicationContext *ijkio_app_ctx;
    int auto_save_map;
    void *cur_ffmpeg_ctx;
    void *ijk_ctx_map;
    void *opaque;
    char cache_map_path[CACHE_MAP_PATH_MAX_LEN];
};

int ijkio_manager_create(IjkIOManagerContext **ph, void *opaque);
void ijkio_manager_destroy(IjkIOManagerContext *h);
void ijkio_manager_destroyp(IjkIOManagerContext **ph);
int ijkio_manager_set_callback(IjkIOManagerContext *h, void *callback);
void ijkio_manager_will_share_cache_map(IjkIOManagerContext *h);
void ijkio_manager_did_share_cache_map(IjkIOManagerContext *h);
void ijkio_manager_immediate_reconnect(IjkIOManagerContext *h);

int ijkio_manager_io_open(IjkIOManagerContext *h, const char *url, int flags, IjkAVDictionary **options);
int ijkio_manager_io_read(IjkIOManagerContext *h, unsigned char *buf, int size);
int64_t ijkio_manager_io_seek(IjkIOManagerContext *h, int64_t offset, int whence);
int ijkio_manager_io_close(IjkIOManagerContext *h);

#endif  // IJKAVFORMAT_IJKIOMANAGER_H
