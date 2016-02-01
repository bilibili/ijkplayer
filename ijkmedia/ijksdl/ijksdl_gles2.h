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

#ifndef IJKSDL__IJKSDL_GLES2_H
#define IJKSDL__IJKSDL_GLES2_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

typedef struct SDL_VoutOverlay SDL_VoutOverlay;

/*
 * Common
 */

#define IJK_GLES2_checkError_TRACE(op) IJK_GLES2_checkError(op)
#define IJK_GLES2_checkError_DEBUG(op) IJK_GLES2_checkError(op)

void IJK_GLES2_printString(const char *name, GLenum s);
void IJK_GLES2_checkError(const char *op);

GLuint IJK_GLES2_loadShader(GLenum shader_type, const char *shader_source);


/*
 * Renderer
 */
#define IJK_GLES2_MAX_PLANE 3
typedef struct IJK_GLES2_Renderer IJK_GLES2_Renderer;
typedef struct IJK_GLES2_Renderer
{
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

    GLsizei buffer_width;
    GLsizei visible_width;
    GLfloat texcoords[8];
} IJK_GLES2_Renderer;

IJK_GLES2_Renderer *IJK_GLES2_Renderer_create(SDL_VoutOverlay *overlay);
void      IJK_GLES2_Renderer_reset(IJK_GLES2_Renderer *renderer);
void      IJK_GLES2_Renderer_free(IJK_GLES2_Renderer *renderer);
void      IJK_GLES2_Renderer_freeP(IJK_GLES2_Renderer **renderer);

GLboolean IJK_GLES2_Renderer_setupGLES();
GLboolean IJK_GLES2_Renderer_isValid(IJK_GLES2_Renderer *renderer);
GLboolean IJK_GLES2_Renderer_use(IJK_GLES2_Renderer *renderer);
GLboolean IJK_GLES2_Renderer_renderOverlay(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay);

#endif
