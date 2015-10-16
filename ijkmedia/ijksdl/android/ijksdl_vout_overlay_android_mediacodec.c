/*****************************************************************************
 * ijksdl_vout_overlay_android_mediacodec.c
 *****************************************************************************
 *
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_vout_overlay_android_mediacodec.h"
#include "ijksdl/ijksdl_stdinc.h"
#include "ijksdl/ijksdl_mutex.h"
#include "ijksdl/ijksdl_vout_internal.h"
#include "ijksdl/ijksdl_video.h"
#include "ijksdl_vout_android_nativewindow.h"
#include "ijksdl_codec_android_mediacodec.h"
#include "ijksdl_inc_internal_android.h"

#ifndef AMCTRACE
#define AMCTRACE(...)
#endif

typedef struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;

    SDL_Vout                   *vout;
    SDL_AMediaCodec            *acodec;

    SDL_AMediaCodecBufferProxy *buffer_proxy;
} SDL_VoutOverlay_Opaque;

static int overlay_lock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_LockMutex(opaque->mutex);
}

static int overlay_unlock(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_UnlockMutex(opaque->mutex);
}

static void overlay_unref(SDL_VoutOverlay *overlay)
{
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;

    SDL_VoutAndroid_releaseBufferProxyP(opaque->vout, &opaque->buffer_proxy, false);
}

static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    AMCTRACE("SDL_Overlay(ffmpeg): overlay_free_l(%p)\n", overlay);
    if (!overlay)
        return;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque)
        return;

    overlay_unref(overlay);

    if (opaque->mutex)
        SDL_DestroyMutex(opaque->mutex);

    SDL_VoutOverlay_FreeInternal(overlay);
}

static SDL_Class g_vout_overlay_amediacodec_class = {
    .name = "AndroidMediaCodecVoutOverlay",
};

inline static bool check_object(SDL_VoutOverlay* object, const char *func_name)
{
    if (!object || !object->opaque || !object->opaque_class) {
        ALOGE("%s.%s: invalid pipeline\n", object->opaque_class->name, func_name);
        return false;
    }

    if (object->opaque_class != &g_vout_overlay_amediacodec_class) {
        ALOGE("%s.%s: unsupported method\n", object->opaque_class->name, func_name);
        return false;
    }

    return true;
}

SDL_VoutOverlay *SDL_VoutAMediaCodec_CreateOverlay(int width, int height, Uint32 format, SDL_Vout *vout)
{
    SDLTRACE("SDL_VoutFFmpeg_CreateOverlay(w=%d, h=%d, fmt=%.4s(0x%x, vout=%p)\n",
        width, height, (const char*) &format, format, vout);
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay) {
        ALOGE("overlay allocation failed");
        return NULL;
    }

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    opaque->mutex         = SDL_CreateMutex();
    opaque->vout          = vout;
    opaque->acodec        = NULL;
    opaque->buffer_proxy  = NULL;

    overlay->opaque_class = &g_vout_overlay_amediacodec_class;
    overlay->format       = format;
    overlay->pitches      = NULL;
    overlay->pixels       = NULL;
    overlay->w            = width;
    overlay->h            = height;
    overlay->free_l       = overlay_free_l;
    overlay->lock         = overlay_lock;
    overlay->unlock       = overlay_unlock;
    overlay->unref        = overlay_unref;

    switch (format) {
    case SDL_FCC__AMC: {
        break;
    }
    default:
        ALOGE("SDL_VoutAMediaCodec_CreateOverlay(...): unknown format %.4s(0x%x)\n", (char*)&format, format);
        goto fail;
    }

    return overlay;

fail:
    overlay_free_l(overlay);
    return NULL;
}

bool SDL_VoutOverlayAMediaCodec_isKindOf(SDL_VoutOverlay *overlay)
{
    return check_object(overlay, __func__);
}

int SDL_VoutOverlayAMediaCodec_attachFrame(SDL_VoutOverlay *overlay, int output_buffer_index)
{
    AMCTRACE("%s(%p, %p, %d)\n", __func__, overlay, acodec, output_buffer_index);
    if (!check_object(overlay, __func__))
        return -1;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (opaque->buffer_proxy)
        SDL_VoutAndroid_releaseBufferProxyP(opaque->vout, &opaque->buffer_proxy, false);

    opaque->acodec       = SDL_VoutAndroid_peekAMediaCodec(opaque->vout);
    opaque->buffer_proxy = SDL_VoutAndroid_obtainBufferProxy(opaque->vout, output_buffer_index);
    return 0;
}

int  SDL_VoutOverlayAMediaCodec_releaseFrame_l(SDL_VoutOverlay *overlay, SDL_AMediaCodec *acodec, bool render)
{
    if (!check_object(overlay, __func__))
        return -1;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return SDL_VoutAndroid_releaseBufferProxyP_l(opaque->vout, &opaque->buffer_proxy, render);
}
