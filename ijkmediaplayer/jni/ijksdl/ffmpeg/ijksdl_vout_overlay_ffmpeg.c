/*****************************************************************************
 * ijksdl_vout_overlay_ffmpeg.c
 *****************************************************************************
 *
 * copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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

#include "ijksdl_vout_overlay_ffmpeg.h"

#include "../ijksdl_stdinc.h"
#include "../ijksdl_mutex.h"
#include "../ijksdl_vout_internal.h"
#include "ijksdl_inc_ffmpeg.h"

typedef struct SDL_VoutOverlay_Opaque {
    SDL_mutex *mutex;

    uint8_t *frame_buf;
    AVFrame *frame;

    Uint16 pitches[AV_NUM_DATA_POINTERS];
    Uint8 *pixels[AV_NUM_DATA_POINTERS];
} SDL_VoutOverlay_Opaque;

/* Always assume a linesize alignment of 1 here */
// TODO: 9 alignment to speed up memcpy when display
static AVFrame *alloc_avframe(SDL_VoutOverlay_Opaque* opaque, enum AVPixelFormat format, int width, int height)
{
    int frame_bytes = avpicture_get_size(format, width, height);
    uint8_t* frame_buf = (uint8_t*) av_malloc(frame_bytes);
    if (!frame_buf)
        return NULL;

    AVFrame *frame = avcodec_alloc_frame();
    if (!frame) {
        av_free(frame_buf);
        return NULL;
    }

    AVPicture *pic = (AVPicture *) frame;
    avcodec_get_frame_defaults(frame);
    avpicture_fill(pic, frame_buf, format, width, height);
    opaque->frame_buf = frame_buf;
    return frame;
}

static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    ALOGE("SDL_Overlay(ffmpeg): overlay_free_l(%p)", overlay);
    if (!overlay)
        return;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque)
        return;

    if (opaque->frame)
        avcodec_free_frame(&opaque->frame);

    if (opaque->frame_buf)
        av_free(opaque->frame_buf);

    if (opaque->mutex)
        SDL_DestroyMutex(opaque->mutex);

    SDL_VoutOverlay_FreeInternal(overlay);
}

static void overlay_fill(SDL_VoutOverlay *overlay, AVFrame *frame, Uint32 format, int planes)
{
    AVPicture *pic = (AVPicture *) frame;
    overlay->planes = planes;

    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i) {
        overlay->pixels[i] = pic->data[i];
        overlay->pitches[i] = pic->linesize[i];
    }
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

SDL_VoutOverlay *SDL_VoutCreateFFmpegOverlay(int width, int height, enum AVPixelFormat format, SDL_Vout *display)
{
    SDLTRACE("SDL_VoutCreateFFmpegOverlay(w=%d, h=%d, fmt=%s(0x%x), dp=%p)",
        width, height, av_get_pix_fmt_name(format), (int)format, display);
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay) {
        ALOGE("SDL_VoutCreateFFmpegOverlay(...)=NULL");
        return NULL;
    }

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    overlay->format = format;
    overlay->pitches = opaque->pitches;
    overlay->pixels = opaque->pixels;
    overlay->w = width;
    overlay->h = height;

    switch (format) {
    case AV_PIX_FMT_YUV420P:
        opaque->frame = alloc_avframe(opaque, AV_PIX_FMT_YUV420P, width, height);
        if (opaque->frame) {
            overlay_fill(overlay, opaque->frame, format, 3);
        }
        overlay->format = SDL_FCC_I420;
        break;
    default:
        ALOGE("SDL_VoutCreateFFmpegOverlay(...): unknown format");
        overlay->format = SDL_FCC_UNDF;
        break;
    }

    if (overlay) {
        SDLTRACE("SDL_VoutCreateFFmpegOverlay(...): overlay(w=%d, h=%d, fmt=%.4s(0x%x), planes=%d, pitches=%d,%d,%d)",
            overlay->w,
            overlay->h,
            (char*)&overlay->format,
            overlay->format,
            overlay->planes,
            overlay->pitches[0],
            overlay->pitches[1],
            overlay->pitches[2]);

        if (opaque->frame) {
            opaque->mutex = SDL_CreateMutex();

            overlay->free_l = overlay_free_l;
            overlay->lock = overlay_lock;
            overlay->unlock = overlay_unlock;
        } else {
            overlay_free_l(overlay);
            overlay = NULL;
        }
    }

    SDLTRACE("SDL_VoutCreateFFmpegOverlay(...)=%p", overlay);
    return overlay;
}

SDL_VoutOverlay *SDL_VoutCreateFFmpegYUVOverlay(int width, int height, Uint32 format, SDL_Vout *display)
{
    SDLTRACE("SDL_VoutCreateFFmpegYUVOverlay(w=%d, h=%d, fmt=%.4s(0x%x, dp=%p)",
        width, height, (const char*) &format, format, display);

    SDL_VoutOverlay *overlay = NULL;
    switch (format) {
    case SDL_FCC_YV12: {
        overlay = SDL_VoutCreateFFmpegOverlay(width, height, AV_PIX_FMT_YUV420P, display);
        if (overlay) {
            /* swap U,V */
            FFSWAP(Uint8*, overlay->pixels[1], overlay->pixels[2]);
            FFSWAP(Uint16, overlay->pitches[1], overlay->pitches[2]);
            overlay->format = SDL_FCC_YV12;
        }
        break;
    }
    case SDL_FCC_RGBP: {
        break;
    }
    default:
        ALOGE("SDL_VoutCreateFFmpegYUVOverlay(...): unknown format");
        break;
    }

    if (!overlay) {
        ALOGE("SDL_VoutCreateFFmpegYUVOverlay(...)=NULL");
    }

    return overlay;
}
