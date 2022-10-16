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
#ifndef IJKAVUTIL_IJKSTL_H
#define IJKAVUTIL_IJKSTL_H

#include <stdint.h>

void* ijk_map_create();
void ijk_map_put(void *data, int64_t key, void *value);
void* ijk_map_get(void *data, int64_t key);
int ijk_map_remove(void *data, int64_t key);
int ijk_map_size(void *data);
int ijk_map_max_size(void *data);
void* ijk_map_index_get(void *data, int index);
void ijk_map_traversal_handle(void *data, void *parm, int (*enu)(void *parm, int64_t key, void *elem));
int64_t ijk_map_get_min_key(void *data);
void ijk_map_clear(void *data);
void ijk_map_destroy(void *data);

#endif /* IJKAVUTIL_IJKSTL_H */
