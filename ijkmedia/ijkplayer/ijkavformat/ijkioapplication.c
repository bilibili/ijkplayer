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
#include "ijkioapplication.h"

#include <stdlib.h>

int  ijkio_application_alloc(IjkIOApplicationContext **ph, void *opaque) {
    IjkIOApplicationContext *h = NULL;

    h = calloc(1, sizeof(IjkIOApplicationContext));
    if (!h)
        return -1;

    h->opaque = opaque;

    *ph = h;
    return 0;
}

int  ijkio_application_open(IjkIOApplicationContext **ph, void *opaque) {
    int ret = ijkio_application_alloc(ph, opaque);
    if (ret)
        return ret;

    return 0;
}

void ijkio_application_close(IjkIOApplicationContext *h) {
    free(h);
}

void ijkio_application_closep(IjkIOApplicationContext **ph) {
    if (!ph || !*ph)
        return;

    ijkio_application_close(*ph);
    *ph = NULL;
}

void ijkio_application_on_cache_statistic(IjkIOApplicationContext *h, IjkIOAppCacheStatistic *statistic) {
    if (h && h->func_ijkio_on_app_event)
        h->func_ijkio_on_app_event(h, IJKIOAPP_EVENT_CACHE_STATISTIC, (void *)statistic, sizeof(IjkIOAppCacheStatistic));
}
