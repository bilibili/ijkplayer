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
#include "ijksdl_codec_android_mediacodec.h"
#include "ijksdl_inc_internal_android.h"
#include "../ijksdl_stdinc.h"
#include "../ijksdl_mutex.h"
#include "../ijksdl_vout_internal.h"
#include "../ijksdl_video.h"

typedef struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;

    SDL_AMediaCodec          *acodec;
    int                       buffer_index;
    SDL_AMediaCodecBufferInfo buffer_info;
    volatile bool             is_buffer_own;
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
    // TODO: error handle
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (opaque->is_buffer_own) {
        SDL_AMediaCodec_releaseOutputBuffer(opaque->acodec, opaque->buffer_index, false);
        SDL_AMediaCodec_decreaseReferenceP(&opaque->acodec);
        opaque->is_buffer_own = false;
    }
}

static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    ALOGE("SDL_Overlay(ffmpeg): overlay_free_l(%p)\n", overlay);
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

int SDL_VoutOverlayAMediaCodec_attachFrame(
    SDL_VoutOverlay *overlay,
    SDL_AMediaCodec *acodec,
    int output_buffer_index,
    SDL_AMediaCodecBufferInfo *buffer_info)
{
    if (!check_object(overlay, __func__))
        return -1;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    opaque->acodec        = acodec;
    opaque->buffer_index  = output_buffer_index;
    opaque->buffer_info   = *buffer_info;
    opaque->is_buffer_own = true;

    SDL_AMediaCodec_increaseReference(acodec);
    return 0;
}

int SDL_VoutOverlayAMediaCodec_releaseFrame(SDL_VoutOverlay *overlay, SDL_AMediaCodec *acodec, bool render)
{
    if (!check_object(overlay, __func__))
        return -1;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (acodec == NULL) {
        acodec = opaque->acodec;
    } else if (acodec != opaque->acodec) {
        ALOGE("%s: mismatch amediacodec orig:%p real:%p\n", __func__, opaque->acodec, acodec);
        return -1;
    }

    if (opaque->buffer_index < 0) {
        // release fake picture buffer
        opaque->is_buffer_own = false;
    } else if (opaque->is_buffer_own) {
        sdl_amedia_status_t amc_ret = SDL_AMediaCodec_releaseOutputBuffer(acodec, opaque->buffer_index, render);
        SDL_AMediaCodec_decreaseReferenceP(&opaque->acodec);
        opaque->is_buffer_own = false;
        if (amc_ret != SDL_AMEDIA_OK) {
            ALOGE("%s: SDL_AMediaCodec_releaseOutputBuffer: failed (%d)\n", __func__, (int)amc_ret);
            return -1;
        }
    }

    return 0;
}
