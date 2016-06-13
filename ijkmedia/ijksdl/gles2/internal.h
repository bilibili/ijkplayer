/*
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

#ifndef IJKSDL__IJKSDL_GLES2__INTERNAL__H
#define IJKSDL__IJKSDL_GLES2__INTERNAL__H

#include <assert.h>
#include <stdlib.h>
#include "ijksdl/ijksdl_fourcc.h"
#include "ijksdl/ijksdl_log.h"
#include "ijksdl/ijksdl_gles2.h"
#include "ijksdl/ijksdl_vout.h"

#define IJK_GLES_STRINGIZE(x)   #x
#define IJK_GLES_STRINGIZE2(x)  IJK_GLES_STRINGIZE(x)
#define IJK_GLES_STRING(x)      IJK_GLES_STRINGIZE2(x)

typedef struct IJK_GLES2_Renderer_Opaque IJK_GLES2_Renderer_Opaque;

typedef struct IJK_GLES2_Renderer
{
    IJK_GLES2_Renderer_Opaque *opaque;

    GLuint program;

    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint plane_textures[IJK_GLES2_MAX_PLANE];

    GLuint av4_position;
    GLuint av2_texcoord;
    GLuint um4_mvp;

    GLuint us2_sampler[IJK_GLES2_MAX_PLANE];
    GLuint um3_color_conversion;

    GLboolean (*func_use)(IJK_GLES2_Renderer *renderer);
    GLsizei   (*func_getBufferWidth)(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay);
    GLboolean (*func_uploadTexture)(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay);
    GLvoid    (*func_destroy)(IJK_GLES2_Renderer *renderer);

    GLsizei buffer_width;
    GLsizei visible_width;

    GLfloat texcoords[8];

    GLfloat vertices[8];
    int     vertices_changed;

    int     format;
    int     gravity;
    GLsizei layer_width;
    GLsizei layer_height;
    int     frame_width;
    int     frame_height;
    int     frame_sar_num;
    int     frame_sar_den;

    GLsizei last_buffer_width;
} IJK_GLES2_Renderer;

typedef struct IJK_GLES_Matrix
{
    GLfloat m[16];
} IJK_GLES_Matrix;
void IJK_GLES2_loadOrtho(IJK_GLES_Matrix *matrix, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

const char *IJK_GLES2_getVertexShader_default();
const char *IJK_GLES2_getFragmentShader_yuv420p();
const char *IJK_GLES2_getFragmentShader_yuv444p10le();
const char *IJK_GLES2_getFragmentShader_yuv420sp();
const char *IJK_GLES2_getFragmentShader_rgb();

const GLfloat *IJK_GLES2_getColorMatrix_bt709();
const GLfloat *IJK_GLES2_getColorMatrix_bt601();

IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_base(const char *fragment_shader_source);
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_yuv420p();
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_yuv444p10le();
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_yuv420sp();
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_yuv420sp_vtb(SDL_VoutOverlay *overlay);
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_rgb565();
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_rgb888();
IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_rgbx8888();

#endif
