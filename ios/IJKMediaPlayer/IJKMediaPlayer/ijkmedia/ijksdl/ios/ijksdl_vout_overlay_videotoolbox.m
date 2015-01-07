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
#include "IJKVideoToolBox.h"


typedef struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;
    CVBufferRef pixBuffer;
    Uint16 pitches[AV_NUM_DATA_POINTERS];
    Uint8 *pixels[AV_NUM_DATA_POINTERS];
} SDL_VoutOverlay_Opaque;


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
    CVBufferRef pixBuffer = CVBufferRetain(picture->cvBufferRef);
    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (opaque->pixBuffer != NULL) {
        CVBufferRelease(opaque->pixBuffer);
    }
    opaque->pixBuffer = pixBuffer;
    overlay->format = SDL_FCC_NV12;
    overlay->planes = 2;

    if (CVPixelBufferLockBaseAddress(pixBuffer, 0) != kCVReturnSuccess) {
        overlay->pixels[0]  = NULL;
        overlay->pixels[1]  = NULL;
        overlay->pitches[0] = 0;
        overlay->pitches[1] = 0;
        overlay->w = 0;
        overlay->h = 0;
        CVBufferRelease(pixBuffer);
        opaque->pixBuffer = NULL;
        return -1;
    }
    overlay->pixels[0]  = CVPixelBufferGetBaseAddressOfPlane(pixBuffer, 0);
    overlay->pixels[1]  = CVPixelBufferGetBaseAddressOfPlane(pixBuffer, 1);
    overlay->pitches[0] = CVPixelBufferGetBytesPerRowOfPlane(pixBuffer, 0);
    overlay->pitches[1] = CVPixelBufferGetBytesPerRowOfPlane(pixBuffer, 1);
    overlay->w = (int)picture->width;
    overlay->h = (int)picture->height;
    CVPixelBufferUnlockBaseAddress(pixBuffer, 0);

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

    CVBufferRelease(opaque->pixBuffer);

    opaque->pixBuffer = NULL;
    overlay->pixels[0] = NULL;
    overlay->pixels[1] = NULL;

    return;
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

fail:
    overlay_free_l(overlay);
    return NULL;
}
