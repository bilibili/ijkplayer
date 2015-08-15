/*****************************************************************************
 * ijksdl_vout_overlay_videotoolbox.m
 *****************************************************************************
 *
 * copyright (c) 2014 ZhouQuan <zhouqicy@gmail.com>
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

#include "ijksdl_vout_overlay_videotoolbox.h"

#include <assert.h>
#include "ijksdl_stdinc.h"
#include "ijksdl_mutex.h"
#include "ijksdl_vout_internal.h"
#include "ijksdl_video.h"


struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;
    CVPixelBufferRef pixel_buffer;
    Uint16 pitches[AV_NUM_DATA_POINTERS];
    Uint8 *pixels[AV_NUM_DATA_POINTERS];
};


static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return;
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque)
        return;
    overlay->unref(overlay);
    if (opaque->mutex)
        SDL_DestroyMutex(opaque->mutex);

    SDL_VoutOverlay_FreeInternal(overlay);
}


int SDL_VoutOverlayVideoToolBox_FillFrame(SDL_VoutOverlay *overlay, VTBPicture* picture)
{
    CVBufferRef pixel_buffer = CVBufferRetain(picture->cvBufferRef);
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (opaque->pixel_buffer != NULL) {
        CVBufferRelease(opaque->pixel_buffer);
    }
    opaque->pixel_buffer = pixel_buffer;
    overlay->format = SDL_FCC_NV12;
    overlay->planes = 2;

#if 0
    if (CVPixelBufferLockBaseAddress(pixel_buffer, 0) != kCVReturnSuccess) {
        overlay->pixels[0]  = NULL;
        overlay->pixels[1]  = NULL;
        overlay->pitches[0] = 0;
        overlay->pitches[1] = 0;
        overlay->w = 0;
        overlay->h = 0;
        CVBufferRelease(pixel_buffer);
        opaque->pixel_buffer = NULL;
        return -1;
    }
    overlay->pixels[0]  = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 0);
    overlay->pixels[1]  = CVPixelBufferGetBaseAddressOfPlane(pixel_buffer, 1);
    overlay->pitches[0] = CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 0);
    overlay->pitches[1] = CVPixelBufferGetBytesPerRowOfPlane(pixel_buffer, 1);
    CVPixelBufferUnlockBaseAddress(pixel_buffer, 0);
#else
    overlay->pixels[0]  = NULL;
    overlay->pixels[1]  = NULL;
    overlay->pitches[0] = 0;
    overlay->pitches[1] = 0;
    overlay->is_private = 1;
#endif

    overlay->w = (int)picture->width;
    overlay->h = (int)picture->height;
    return 0;
}

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
    if (!overlay) {
        return;
    }
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque) {
        return;
    }

    CVBufferRelease(opaque->pixel_buffer);

    opaque->pixel_buffer = NULL;
    overlay->pixels[0] = NULL;
    overlay->pixels[1] = NULL;

    return;
}

static SDL_Class g_vout_overlay_videotoolbox_class = {
    .name = "VideoToolboxVoutOverlay",
};

static bool check_object(SDL_VoutOverlay* object, const char *func_name)
{
    if (!object || !object->opaque || !object->opaque_class) {
        ALOGE("%s: invalid pipeline\n", func_name);
        return false;
    }

    if (object->opaque_class != &g_vout_overlay_videotoolbox_class) {
        ALOGE("%s.%s: unsupported method\n", object->opaque_class->name, func_name);
        return false;
    }

    return true;
}

CVPixelBufferRef SDL_VoutOverlayVideoToolBox_GetCVPixelBufferRef(SDL_VoutOverlay *overlay)
{
    if (!check_object(overlay, __func__))
        return NULL;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    return opaque->pixel_buffer;
}

SDL_VoutOverlay *SDL_VoutVideoToolBox_CreateOverlay(int width, int height, Uint32 format, SDL_Vout *display)
{
    SDLTRACE("SDL_VoutVideoToolBox_CreateOverlay(w=%d, h=%d, fmt=%.4s(0x%x, dp=%p)\n", width, height, (const char*) &format, format, display);
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay) {
        ALOGE("overlay allocation failed");
        return NULL;
    }
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    overlay->opaque_class = &g_vout_overlay_videotoolbox_class;
    overlay->format = format;
    overlay->w = width;
    overlay->h = height;
    overlay->pitches = opaque->pitches;
    overlay->pixels = opaque->pixels;
    overlay->free_l = overlay_free_l;
    overlay->lock = overlay_lock;
    overlay->unlock = overlay_unlock;
    overlay->unref = overlay_unref;
    opaque->mutex = SDL_CreateMutex();
    return overlay;
}
