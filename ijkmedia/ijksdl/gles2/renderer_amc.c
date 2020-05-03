/*
 * copyright (c) 2020 Befovy <befovy@gmail.com>
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

#include "ijksdl/android/ijksdl_vout_overlay_android_mediacodec.h"
#include "internal.h"
#include "ijksdl/android/ijksdl_android_jni.h"
#include "ijkj4a/j4a/j4a_base.h"
#include "ijkj4a/j4a/j4a_allclasses.h"

static SDL_Class g_class = {
        .name = "amc_renderer",
};

typedef struct IJK_GLES2_Renderer_Opaque
{
    SDL_Class *class;
    jobject  amc_surface;
    jfloatArray mtx;
} IJK_GLES2_Renderer_Opaque;


static GLboolean amc_use(IJK_GLES2_Renderer *renderer)
{
    ALOGI("use render amc\n");
    IJK_GLES2_Renderer_Opaque* opaque = renderer->opaque;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glUseProgram(renderer->program);            IJK_GLES2_checkError_TRACE("glUseProgram");

    if (0 == renderer->plane_textures[0])
        glGenTextures(1, renderer->plane_textures);


    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_Android_GetApiLevel: SetupThreadEnv failed");
        return 0;
    }

    // ALOGI("SurfaceTexture: detachFromGLContext");
    // J4AC_MediaCodecSurface__detachFromGLContext(env, opaque->amc_surface);
    // ALOGI("SurfaceTexture: detachFromGLContext=void");

    if (opaque->mtx == NULL) {
        jfloatArray mtx = (*env)->NewFloatArray(env, 16);
        opaque->mtx = J4A_NewGlobalRef__catchAll(env, mtx);
    }

    for (int i = 0; i < 1; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, renderer->plane_textures[i]);


        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glUniform1i(renderer->us2_sampler[i], i);
    }

    ALOGI("SurfaceTexture: attachToGLContext");
    J4AC_MediaCodecSurface__attachToGLContext(env, opaque->amc_surface,
                                              renderer->plane_textures[0], 0, 0);
    ALOGI("SurfaceTexture: attachToGLContext=void");

    return GL_TRUE;
}


static GLsizei amc_getBufferWidth(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return 0;
    return overlay->w;
}

static GLboolean amc_uploadTexture(IJK_GLES2_Renderer *renderer, SDL_VoutOverlay *overlay)
{
    if (!renderer || !overlay)
        return GL_FALSE;

    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_Android_GetApiLevel: SetupThreadEnv failed");
        return 0;
    }
    IJK_GLES2_Renderer_Opaque* opaque = renderer->opaque;

    switch (overlay->format) {
        case SDL_FCC__AMC:
            SDL_VoutOverlayAMediaCodec_releaseFrame_l(overlay, NULL, true);
            J4AC_MediaCodecSurface__updateTexImage(env, opaque->amc_surface, opaque->mtx);
            jboolean isCopy = false;
            jfloat *mat = (*env)->GetFloatArrayElements(env, opaque->mtx, &isCopy);
            glUniformMatrix4fv(renderer->um4_mat, 1, GL_FALSE, mat);
            (*env)->ReleaseFloatArrayElements(env, opaque->mtx, mat, 0);
            break;
        default:
            ALOGE("[amc] unexpected format %x\n", overlay->format);
            return GL_FALSE;
    }
    return GL_TRUE;
}

static void amc_flip(int flip, GLfloat *texcoords)
{
    GLfloat tmp;
    switch (flip) {
        case IJK_SDL_GLES2_flip_none:
            tmp = texcoords[0];
            texcoords[0] = texcoords[4];
            texcoords[4] = tmp;

            tmp = texcoords[1];
            texcoords[1] = texcoords[5];
            texcoords[5] = tmp;

            tmp = texcoords[2];
            texcoords[2] = texcoords[6];
            texcoords[6] = tmp;

            tmp = texcoords[3];
            texcoords[3] = texcoords[7];
            texcoords[7] = tmp;
            break;
        case IJK_SDL_GLES2_flip_both:
            tmp = texcoords[0];
            texcoords[0] = texcoords[2];
            texcoords[4] = tmp;

            tmp = texcoords[1];
            texcoords[1] = texcoords[3];
            texcoords[3] = tmp;

            tmp = texcoords[4];
            texcoords[4] = texcoords[6];
            texcoords[6] = tmp;

            tmp = texcoords[5];
            texcoords[5] = texcoords[7];
            texcoords[7] = tmp;
            break;
        case IJK_SDL_GLES2_flip_horizontal:
            tmp = texcoords[0];
            texcoords[0] = texcoords[6];
            texcoords[6] = tmp;

            tmp = texcoords[1];
            texcoords[1] = texcoords[7];
            texcoords[7] = tmp;

            tmp = texcoords[4];
            texcoords[4] = texcoords[2];
            texcoords[2] = tmp;

            tmp = texcoords[5];
            texcoords[5] = texcoords[3];
            texcoords[3] = tmp;
            break;
        default:
            break;
    }
}

void IJK_GLES2_Renderer_AMC_set_texture(IJK_GLES2_Renderer *renderer, jobject  amc_surface)
{
    if (!renderer)
        return;
    IJK_GLES2_Renderer_Opaque* opaque = renderer->opaque;
    if (opaque->class == &g_class) {
        opaque->amc_surface = amc_surface;
    }
}

static  void amc_destroy(IJK_GLES2_Renderer *renderer)
{
    IJK_GLES2_Renderer_Opaque* opaque = renderer->opaque;
    JNIEnv *env = NULL;
    if (JNI_OK != SDL_JNI_SetupThreadEnv(&env)) {
        ALOGE("SDL_Android_GetApiLevel: SetupThreadEnv failed");
        return;
    }
    //if (opaque->amc_surface)
    //    J4AC_MediaCodecSurface__detachFromGLContext(env, opaque->amc_surface);

    J4A_DeleteGlobalRef(env, opaque->mtx);
    // J4AC_MediaCodecSurface__release(env, opaque->amc_surface);
}


IJK_GLES2_Renderer *IJK_GLES2_Renderer_create_amc()
{
    ALOGI("create render gles2 amc\n");
    IJK_GLES2_Renderer *renderer = IJK_GLES2_Renderer_create_base_with_vertex(IJK_GLES2_getVertexShader_amc(), IJK_GLES2_getFragmentShader_amc());
    if (!renderer) {
        goto fail;
    }
    renderer->um4_mat = (GLuint)glGetUniformLocation(renderer->program, "um4_Matrix");
    // IJK_GLES2_checkError_TRACE("glGetUniformLocation(um4_Matrix)");

    renderer->us2_sampler[0] = glGetUniformLocation(renderer->program, "us2_SamplerX");

    renderer->func_use = amc_use;
    renderer->func_getBufferWidth = amc_getBufferWidth;
    renderer->func_uploadTexture = amc_uploadTexture;
    renderer->func_flip = amc_flip;
    renderer->func_destroy = amc_destroy;
    renderer->opaque = mallocz(sizeof(IJK_GLES2_Renderer_Opaque));
    renderer->opaque->class = &g_class;
    return renderer;
fail:
    IJK_GLES2_Renderer_free(renderer);
    return NULL;
}