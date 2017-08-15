/*
 * Copyright (c) 2015 Bilibili
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

#ifndef IJKSDL__IJKSDL_CONTAINER_H
#define IJKSDL__IJKSDL_CONTAINER_H

#include <stdlib.h>

typedef int isdl_error;

/*
 * Array
 */

typedef struct ISDL_Array
{
    void   **elements;
    size_t   capacity;
    size_t   size;
} ISDL_Array;

inline static isdl_error ISDL_Array__grow(ISDL_Array *array, size_t new_capacity)
{
    assert(array);
    if (array->capacity >= new_capacity)
        return 0;

    void **new_elements = realloc(array->elements, sizeof(void *) * new_capacity);
    if (!new_elements)
        return -1;

    array->elements = new_elements;
    array->capacity = new_capacity;
    return 0;
}

inline static isdl_error ISDL_Array__init(ISDL_Array *array, size_t capacity)
{
    assert(array);
    memset(array, 0, sizeof(ISDL_Array));
    if (ISDL_Array__grow(array, capacity))
        return -1;

    return 0;
}

inline static isdl_error ISDL_Array__push_back(ISDL_Array *array, void *val)
{
    assert(array);
    if (array->size >= array->capacity) {
        if (ISDL_Array__grow(array, array->capacity * 2))
            return -1;
    }

    array->elements[array->size++] = val;
    return 0;
}

inline static void *ISDL_Array__pop_back(ISDL_Array *array)
{
    assert(array);
    assert(array->size >= 1);
    return array->elements[--array->size];
}

inline static void ISDL_Array__clear(ISDL_Array *array)
{
    assert(array);
    array->size = 0;
}

inline static void *ISDL_Array__at(ISDL_Array *array, int pos)
{
    assert(array);
    assert(pos >= 0);
    assert(pos < array->size);
    return array->elements[pos];
}

inline static size_t ISDL_Array__size(ISDL_Array *array)
{
    assert(array);
    return array->size;
}

inline static void **ISDL_Array__begin(ISDL_Array *array)
{
    assert(array);
    return array->elements;
}

inline static void **ISDL_Array__end(ISDL_Array *array)
{
    assert(array);
    return array->elements + array->size;
}

inline static void *ISDL_Array__back(ISDL_Array *array)
{
    assert(array);
    assert(array->size >= 1);
    return array->elements[array->size - 1];
}

#endif
