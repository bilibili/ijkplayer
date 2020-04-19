/*****************************************************************************
 * ijksdl_log.c
 *****************************************************************************
 *
 * Copyright (c) 2020 Befovy
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

#include "ijksdl_log.h"

#if ANDROID
#include <android/log.h>
#endif


#define LOG_BUF_SIZE	1024

static int g_ijksdl_log_level = 0;

void ijk_log_set_level(int level)
{
    g_ijksdl_log_level = level;
}

void ijk_log_print(int level, const char *tag, const char *fmt, ...)
{
    if (level < g_ijksdl_log_level)
        return;

    va_list ap;
    va_start(ap, fmt);
    ijk_log_vprint(level, tag, fmt, ap);
    va_end(ap);
}

void ijk_log_vprint(int level, const char *tag, const char *fmt, va_list ap)
{
    if (level < g_ijksdl_log_level)
        return;

#if ANDROID
    __android_log_vprint(level, tag, fmt, ap);
#else
    vprintf(fmt, ap);
#endif
}
