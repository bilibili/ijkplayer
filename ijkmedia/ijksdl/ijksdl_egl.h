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

#ifndef IJKSDL__IJKSDL_EGL_H
#define IJKSDL__IJKSDL_EGL_H

#ifdef __APPLE__
#include "ijksdl/ios/EGL/egl.h"
#include "ijksdl/ios/EGL/eglplatform.h"
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#endif
#include "ijksdl_class.h"

typedef struct SDL_VoutOverlay SDL_VoutOverlay;
typedef struct IJK_EGL_Opaque  IJK_EGL_Opaque;

#if 0
enum {
    IJK_GLES2__GL_EXT_texture_rg,
    IJK_GLES2__MAX_EXT,
};
#endif

typedef struct IJK_EGL
{
    SDL_Class      *opaque_class;
    IJK_EGL_Opaque *opaque;

    EGLNativeWindowType window;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    EGLint width;
    EGLint height;

#if 0
    uint8_t gles2_extensions[IJK_GLES2__MAX_EXT];
#endif
} IJK_EGL;

IJK_EGL    *IJK_EGL_create();
void        IJK_EGL_free(IJK_EGL *egl);
void        IJK_EGL_freep(IJK_EGL **egl);

EGLBoolean  IJK_EGL_display(IJK_EGL* egl, EGLNativeWindowType window, SDL_VoutOverlay *overlay);
void        IJK_EGL_terminate(IJK_EGL* egl);

#endif
