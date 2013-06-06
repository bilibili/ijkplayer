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

#ifndef IJKUTIL__LOGHELP_H
#define IJKUTIL__LOGHELP_H

#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "IJKMEDIA"
#endif

#define ALOG(level, TAG, ...)    ((void)__android_log_print(level, TAG, __VA_ARGS__))
#define ALOGV(...)  ALOG(ANDROID_LOG_VERBOSE,   LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(ANDROID_LOG_DEBUG,     LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(ANDROID_LOG_INFO,      LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  ALOG(ANDROID_LOG_WARN,      LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(ANDROID_LOG_ERROR,     LOG_TAG, __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)

#endif

