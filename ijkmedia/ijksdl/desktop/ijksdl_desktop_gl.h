/*****************************************************************************
* ijksdl_vout_callback.c
*****************************************************************************
*
* copyright (c) 2020 befovy <befovy@gmail.com>
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


#ifndef IJKPLAYER_IJKSDL_DESKTOP_GL_H
#define IJKPLAYER_IJKSDL_DESKTOP_GL_H

#include "ijksdl_class.h"
#include "../ijksdl_gles2.h"

typedef struct SDL_VoutOverlay SDL_VoutOverlay;
typedef struct IJK_GL_Opaque  IJK_GL_Opaque;

typedef struct IJK_GL
{
    SDL_Class      *opaque_class;
    IJK_GL_Opaque *opaque;

    GLFWwindow *window;
    GLboolean  attached;

    GLint width;
    GLint height;

} IJK_GL;


IJK_GL    *IJK_GL_create();
void       IJK_GL_free(IJK_GL *gl);
void       IJK_GL_freep(IJK_GL **gl);


GLboolean  IJK_GL_display(IJK_GL* egl, GLFWwindow *window, SDL_VoutOverlay *overlay);
void       IJK_GL_terminate(IJK_GL* egl);

#endif //IJKPLAYER_IJKSDL_DESKTOP_GL_H
