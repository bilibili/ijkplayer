/*****************************************************************************
 * ijksdl_vout_ffmpeg.c
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

#include "ijksdl_vout_ffmpeg.h"

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "ijksdl_vout_internal.h"

typedef struct SDL_VoutOverlay_Opaque {
    uint8_t *frame_buf;
    AVFrame *frame;

    Uint16 pitches[AV_NUM_DATA_POINTERS];
    Uint8 *pixels[AV_NUM_DATA_POINTERS];
} SDL_VoutOverlay_Opaque;

/* Always assume a linesize alignment of 1 here */
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

    avcodec_get_frame_defaults(frame);
    avpicture_fill((AVPicture *) frame, frame_buf, format, width, height);
    opaque->frame_buf = frame_buf;
    return frame;
}

static void overlay_free_l(SDL_VoutOverlay *overlay)
{
    if (!overlay)
        return;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    if (!opaque)
        return;

    if (opaque->frame)
        avcodec_free_frame(&opaque->frame);

    if (opaque->frame_buf)
        av_free(opaque->frame_buf);

    SDL_VoutOverlay_FreeInternal(overlay);
}

static void overlay_fill(SDL_VoutOverlay *overlay, AVFrame *frame, Uint32 format, int planes)
{
    AVPicture *pic = (AVPicture *) frame;
    overlay->format = format;
    overlay->w = frame->width;
    overlay->h = frame->height;
    overlay->planes = planes;

    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i) {
        overlay->pixels[i] = pic->data[i];
        overlay->pitches[i] = pic->linesize[i];
    }
}

SDL_VoutOverlay *SDL_VoutCreateFFmpegYUVOverlay(int width, int height, Uint32 format, SDL_VoutSurface *display)
{
    SDL_VoutOverlay *overlay = SDL_VoutOverlay_CreateInternal(sizeof(SDL_VoutOverlay_Opaque));
    if (!overlay)
        return NULL;

    overlay->format = format;

    SDL_VoutOverlay_Opaque *opaque = overlay->opaque;
    overlay->pitches = opaque->pitches;
    overlay->pixels = opaque->pixels;

    AVFrame *frame = NULL;
    AVPicture *pic = NULL;
    switch (format) {
    case SDL_YV12_OVERLAY:
        frame = alloc_avframe(opaque, AV_PIX_FMT_YUV420P, width, height);
        overlay_fill(overlay, frame, format, 3);
        /* swap U,V */
        pic = (AVPicture *) frame;
        overlay->pixels[2] = pic->data[1];
        overlay->pixels[1] = pic->data[2];

        overlay->pitches[2] = pic->linesize[1];
        overlay->pitches[1] = pic->linesize[2];
        break;
    default:
        break;
    }

    if (frame) {
        overlay->free_l = overlay_free_l;
    } else {
        SDL_VoutOverlay_FreeInternal(overlay);
        overlay = NULL;
    }

    return overlay;
}
