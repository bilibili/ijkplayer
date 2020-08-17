/*
 * Copyright (c) 2016 Bilibili
 * copyright (c) 2016 Zhang Rui <bbcallen@gmail.com>
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

#include "internal.h"

// BT.709, which is the standard for HDTV.
static const GLfloat g_bt709[] = {
    1.164f,  1.164f,  1.164f,
    0.0f,   -0.213f,  2.112f,
    1.793f, -0.533f,  0.0f,
};
const GLfloat *IJK_GLES2_getColorMatrix_bt709()
{
    return g_bt709;
}

static const GLfloat g_bt601[] = {
    1.164f,  1.164f, 1.164f,
    0.0f,   -0.392f, 2.017f,
    1.596f, -0.813f, 0.0f,
};
const GLfloat *IJK_GLES2_getColorMatrix_bt601()
{
    return g_bt601;
}
