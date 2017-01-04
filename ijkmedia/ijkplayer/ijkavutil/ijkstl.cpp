/*
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
#include <map>

using namespace std;

typedef map<int64_t, void *> IjkMap;

extern "C" void* ijk_map_create();
extern "C" void ijk_map_put(void *data, int64_t key, void *value);
extern "C" void* ijk_map_get(void *data, int64_t key);
extern "C" int ijk_map_remove(void *data, int64_t key);
extern "C" int ijk_map_size(void *data);
extern "C" int ijk_map_max_size(void *data);
extern "C" void* ijk_map_index_get(void *data, int index);
extern "C" int64_t ijk_map_get_min_key(void *data);
extern "C" void ijk_map_clear(void *data);
extern "C" void ijk_map_destroy(void *data);
extern "C" void ijk_map_traversal_handle(void *data, int (*enu)(void *elem));

void* ijk_map_create() {
    IjkMap *data = new IjkMap();
    return data;
}

void ijk_map_put(void *data, int64_t key, void *value) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return;
    (*map_data)[key] = value;
}

void* ijk_map_get(void *data, int64_t key) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return NULL;

    IjkMap::iterator it = map_data->find(key);
    if (it != map_data->end()) {
        return it->second;
    }
    return NULL;
}

int ijk_map_remove(void *data, int64_t key) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return -1;
    map_data->erase(key);
    return 0;
}

int ijk_map_size(void *data) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return 0;

    return map_data->size();
}

int ijk_map_max_size(void *data) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return 0;

    return map_data->max_size();
}

void* ijk_map_index_get(void *data, int index) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data || map_data->empty())
        return NULL;

    IjkMap::iterator it;
    it = map_data->begin();

    for (int i = 0; i < index; i++) {
        it = it++;
        if (it == map_data->end()) {
            return NULL;
        }
    }

    return it->second;
}

void ijk_map_traversal_handle(void *data, int (*enu)(void *elem)) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data || map_data->empty())
        return;

    IjkMap::iterator it;

    for (it = map_data->begin(); it != map_data->end(); it++) {
        enu(it->second);
    }
}

int64_t ijk_map_get_min_key(void *data) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data || map_data->empty())
        return -1;

    IjkMap::iterator it = map_data->begin();

    int64_t min_key = it->first;

    for (; it != map_data->end(); it++) {
        min_key = min_key < it->first ? min_key : it->first;
    }

    return min_key;
}

void ijk_map_clear(void *data) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return;

    map_data->clear();
}

void ijk_map_destroy(void *data) {
    IjkMap *map_data = reinterpret_cast<IjkMap *>(data);
    if (!map_data)
        return;

    map_data->clear();
    delete(map_data);
}
