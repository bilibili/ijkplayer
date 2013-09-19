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

#ifdef __cplusplus
extern "C" {
#endif

#define IJK_LOG_TAG "IJKMEDIA"

#define IJK_LOG_UNKNOWN 0
#define IJK_LOG_DEFAULT 0

#define IJK_LOG_VERBOSE 0
#define IJK_LOG_DEBUG 0
#define IJK_LOG_INFO 0
#define IJK_LOG_WARN 0
#define IJK_LOG_ERROR 0
#define IJK_LOG_FATAL 0
#define IJK_LOG_SILENT 0

#define VLOG(level, TAG, ...)
#define VLOGV(...)
#define VLOGD(...)
#define VLOGI(...)
#define VLOGW(...)
#define VLOGE(...)

#define ALOG(level, TAG, ...)
#define ALOGV(...)
#define ALOGD(...)
#define ALOGI(...)
#define ALOGW(...)
#define ALOGE(...)
#define LOG_ALWAYS_FATAL(...)

#ifdef __cplusplus
}
#endif

#endif
