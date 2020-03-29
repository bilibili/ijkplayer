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

#include "ijksdl_desktop_gl.h"
#include "ijksdl/ijksdl_log.h"
#include "ijksdl/ijksdl_vout.h"
#include "ijksdl/gles2/internal.h"

typedef struct IJK_GL_Opaque {
    IJK_GLES2_Renderer *renderer;
} IJK_GL_Opaque;

static GLboolean IJK_GL_isValid(IJK_GL *gl) {
    if (gl && gl->window && gl->attached) {
        return GL_TRUE;
    }
    return GL_FALSE;
}

static void IJK_GL_getSize(IJK_GL *gl) {
    GLint width = 0;
    GLint height = 0;
    glfwGetFramebufferSize(gl->window, &width, &height);
    gl->width = width;
    gl->height = height;
}

static GLboolean IJK_EGL_makeCurrent(IJK_GL *gl, GLFWwindow *window) {
    if (window && window == gl->window && gl->attached) {
        if (glfwGetCurrentContext() == gl->window)
            return GL_TRUE;
        return GL_FALSE;
    }

    gl->window = window;
    if (!window)
        return GL_FALSE;
    glfwMakeContextCurrent(window);
    IJK_GLES2_Renderer_setupGLES();
    return GL_TRUE;
}

static GLboolean IJK_GL_prepareRenderer(IJK_GL *gl, SDL_VoutOverlay *overlay) {
    assert(gl);
    assert(gl->opaque);

    IJK_GL_Opaque *opaque = gl->opaque;

    if (!IJK_GLES2_Renderer_isValid(opaque->renderer) ||
        !IJK_GLES2_Renderer_isFormat(opaque->renderer, overlay->format)) {
        IJK_GLES2_Renderer_reset(opaque->renderer);
        IJK_GLES2_Renderer_freeP(&opaque->renderer);

        opaque->renderer = IJK_GLES2_Renderer_create(overlay);
        if (!opaque->renderer) {
            ALOGE("[EGL] Could not create render.");
            return GL_FALSE;
        }

        if (!IJK_GLES2_Renderer_use(opaque->renderer)) {
            ALOGE("[EGL] Could not use render.");
            IJK_GLES2_Renderer_freeP(&opaque->renderer);
            return GL_FALSE;
        }
    }
    IJK_GL_getSize(gl);

    glViewport(0, 0, gl->width, gl->height);
    IJK_GLES2_checkError_TRACE("glViewport");
    return GL_TRUE;
}

GLboolean IJK_GL_display(IJK_GL *gl, GLFWwindow *window, SDL_VoutOverlay *overlay) {

    if (!gl || !gl->opaque)
        return GL_FALSE;
    if (!IJK_EGL_makeCurrent(gl, window))
        return GL_FALSE;
    if (!IJK_GL_prepareRenderer(gl, overlay)) {
        ALOGE("[EGL] IJK_EGL_prepareRenderer failed\n");
        return GL_FALSE;
    }
    IJK_GL_Opaque *opaque = gl->opaque;

    if (!IJK_GLES2_Renderer_renderOverlay(opaque->renderer, overlay)) {
        ALOGE("[EGL] IJK_GLES2_render failed\n");
        return GL_FALSE;
    }
    glfwSwapBuffers(window);
    glfwMakeContextCurrent(NULL);

    return GL_TRUE;
}


void IJK_GL_terminate(IJK_GL *gl) {
    if (!IJK_GL_isValid(gl))
        return;
    if (gl->opaque)
        IJK_GLES2_Renderer_free(gl->opaque->renderer);

    glfwMakeContextCurrent(NULL);
    gl->attached = GL_FALSE;
}

void IJK_GL_free(IJK_GL *gl) {
    if (!gl)
        return;

    IJK_GL_terminate(gl);
    memset(gl, 0, sizeof(IJK_GL));
    free(gl);
}

void IJK_GL_freep(IJK_GL **gl) {
    if (!gl || !*gl)
        return;

    IJK_GL_free(*gl);
    *gl = NULL;
}

static SDL_Class gl_class = {
        .name = "GLFW_GL",
};

IJK_GL *IJK_GL_create() {
    IJK_GL *gl = (IJK_GL *) mallocz(sizeof(IJK_GL));
    if (!gl)
        return NULL;

    gl->opaque_class = &gl_class;
    gl->opaque = mallocz(sizeof(IJK_GL_Opaque));
    if (!gl->opaque) {
        free(gl);
        return NULL;
    }
    return gl;
}