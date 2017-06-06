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

static void IJK_GLES2_printShaderInfo(GLuint shader)
{
    if (!shader)
        return;

    GLint info_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
    if (!info_len) {
        ALOGE("[GLES2][Shader] empty info\n");
        return;
    }

    char    buf_stack[32];
    char   *buf_heap = NULL;
    char   *buf      = buf_stack;
    GLsizei buf_len  = sizeof(buf_stack) - 1;
    if (info_len > sizeof(buf_stack)) {
        buf_heap = (char*) malloc(info_len + 1);
        if (buf_heap) {
            buf     = buf_heap;
            buf_len = info_len;
        }
    }

    glGetShaderInfoLog(shader, buf_len, NULL, buf);
    ALOGE("[GLES2][Shader] error %s\n", buf);

    if (buf_heap)
        free(buf_heap);
}

GLuint IJK_GLES2_loadShader(GLenum shader_type, const char *shader_source)
{
    assert(shader_source);

    GLuint shader = glCreateShader(shader_type);        IJK_GLES2_checkError("glCreateShader");
    if (!shader)
        return 0;

    assert(shader_source);

    glShaderSource(shader, 1, &shader_source, NULL);    IJK_GLES2_checkError_TRACE("glShaderSource");
    glCompileShader(shader);                            IJK_GLES2_checkError_TRACE("glCompileShader");

    GLint compile_status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status)
        goto fail;

    return shader;

fail:

    if (shader) {
        IJK_GLES2_printShaderInfo(shader);
        glDeleteShader(shader);
    }

    return 0;
}
