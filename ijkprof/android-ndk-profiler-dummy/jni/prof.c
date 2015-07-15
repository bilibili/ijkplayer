/*****************************************************************************
 * prof.c
 *****************************************************************************
 *
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#include "prof.h"
#include <android/log.h>

extern void monstartup(const char *libname)
{
    __android_log_print(ANDROID_LOG_DEBUG, "aprof-fake", "fake-monstartup %s\n", libname);
}

extern void moncleanup(void)
{
    __android_log_print(ANDROID_LOG_DEBUG, "aprof-fake", "fake-momcleanup\n");
}
