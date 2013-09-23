/*****************************************************************************
 * loghelper.h
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKUTIL_IOS__LOGHELP_H
#define IJKUTIL_IOS__LOGHELP_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IJK_LOG_TAG "IJKMEDIA"

#define IJK_LOG_UNKNOWN     0
#define IJK_LOG_DEFAULT     1

#define IJK_LOG_VERBOSE     2
#define IJK_LOG_DEBUG       3
#define IJK_LOG_INFO        4
#define IJK_LOG_WARN        5
#define IJK_LOG_ERROR       6
#define IJK_LOG_FATAL       7
#define IJK_LOG_SILENT      8

#define VLOG(level, TAG, ...)    ((void)vprintf(__VA_ARGS__))
#define VLOGV(...)  VLOG(IJK_LOG_VERBOSE,   IJK_LOG_TAG, __VA_ARGS__)
#define VLOGD(...)  VLOG(IJK_LOG_DEBUG,     IJK_LOG_TAG, __VA_ARGS__)
#define VLOGI(...)  VLOG(IJK_LOG_INFO,      IJK_LOG_TAG, __VA_ARGS__)
#define VLOGW(...)  VLOG(IJK_LOG_WARN,      IJK_LOG_TAG, __VA_ARGS__)
#define VLOGE(...)  VLOG(IJK_LOG_ERROR,     IJK_LOG_TAG, __VA_ARGS__)

#define ALOG(level, TAG, ...)    ((void)printf(__VA_ARGS__))
#define ALOGV(...)  ALOG(IJK_LOG_VERBOSE,   IJK_LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(IJK_LOG_DEBUG,     IJK_LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(IJK_LOG_INFO,      IJK_LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  ALOG(IJK_LOG_WARN,      IJK_LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(IJK_LOG_ERROR,     IJK_LOG_TAG, __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)

#ifdef __cplusplus
}
#endif

#endif
