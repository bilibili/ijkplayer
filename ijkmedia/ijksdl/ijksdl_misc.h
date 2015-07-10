/*****************************************************************************
 * ijksdl_misc.h
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

#ifndef IJKSDL__IJKSDL_MISC_H
#define IJKSDL__IJKSDL_MISC_H

#include <stdlib.h>
#include <memory.h>

#ifndef IJKMAX
#define IJKMAX(a, b)    ((a) > (b) ? (a) : (b))
#endif

#ifndef IJKMIN
#define IJKMIN(a, b)    ((a) < (b) ? (a) : (b))
#endif

#ifndef IJKALIGN
#define IJKALIGN(x, align) ((( x ) + (align) - 1) / (align) * (align))
#endif

#define IJK_CHECK_RET(condition__, retval__, ...) \
    if (!(condition__)) { \
        ALOGE(__VA_ARGS__); \
        return (retval__); \
    }

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

inline static void *mallocz(size_t size)
{
    void *mem = malloc(size);
    if (!mem)
        return mem;

    memset(mem, 0, size);
    return mem;
}

inline static void freep(void **mem)
{
    if (mem && *mem) {
        free(*mem);
        *mem = NULL;
    }
}

#endif
