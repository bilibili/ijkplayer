/*****************************************************************************
 * ijksdl_extra_log.c
 *****************************************************************************
 *
 * Copyright (c) 2017 Bilibili
 * copyright (c) 2017 Raymond Zheng <raymondzheng1412@gmail.com>
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
#include "ijksdl_extra_log.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include <stdio.h>

#define LOG_BUF_SIZE	1024

#ifdef EXTRA_LOG_PRINT
void ffp_log_extra_print(int level, const char *tag, const char *fmt, ...)
{
    JNIEnv *env = NULL;

    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        return;
    }
    va_list ap;
    char log_buffer[LOG_BUF_SIZE] = {0};

    va_start(ap, fmt);
    vsnprintf(log_buffer, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);
    switch (level) {
        case ANDROID_LOG_UNKNOWN:
        case ANDROID_LOG_DEFAULT:
        case ANDROID_LOG_VERBOSE:
            J4AC_BLog__v__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_DEBUG:
            J4AC_BLog__d__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_INFO:
            J4AC_BLog__i__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_WARN:
            J4AC_BLog__w__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_FATAL:
        case ANDROID_LOG_SILENT:
        case ANDROID_LOG_ERROR:
            J4AC_BLog__e__withCString__catchAll(env, tag, log_buffer);
            break;
        default:
            break;
    }
}

void ffp_log_extra_vprint(int level, const char *tag, const char *fmt, va_list ap)
{
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        return;
    }
    char log_buffer[LOG_BUF_SIZE] = {0};

    vsnprintf(log_buffer, LOG_BUF_SIZE, fmt, ap);
    switch (level) {
        case ANDROID_LOG_UNKNOWN:
        case ANDROID_LOG_DEFAULT:
        case ANDROID_LOG_VERBOSE:
            J4AC_BLog__v__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_DEBUG:
            J4AC_BLog__d__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_INFO:
            J4AC_BLog__i__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_WARN:
            J4AC_BLog__w__withCString__catchAll(env, tag, log_buffer);
            break;
        case ANDROID_LOG_FATAL:
        case ANDROID_LOG_SILENT:
        case ANDROID_LOG_ERROR:
            J4AC_BLog__e__withCString__catchAll(env, tag, log_buffer);
            break;
        default:
            break;
    }
}
#endif  // EXTRA_LOG_PRINT