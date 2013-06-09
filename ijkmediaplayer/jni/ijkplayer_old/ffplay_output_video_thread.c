/*****************************************************************************
 * ffplay_output_video_thread.c
 *****************************************************************************
 *
 * copyright (c) 2001 Fabrice Bellard
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

#if 0
static int video_open(FFPlayer *ffp, int force_set_video_mode, VideoPicture *vp)
{
    VideoState *is = &ffp->is;
    SDL_Vout *vout = ffp->vout;
    int w = 0;
    int h = 0;
    SDL_Rect rect;

    if (!vp)
        return 0;

    if (vp && vp->width && vp->height) {
        w = rect.w;
        h = rect.h;
    }

    if (vout &&
        is->width  == vout->width  && vout->width  == w &&
        is->height == vout->height && vout->height == h &&
        !force_set_video_mode)
        return 0;

    if (SDL_VoutSetBuffersGeometry(vout, w, h, 0))
    {
        ALOGE("SDL_SetBuffersGeometry(%d, %d, 0) failed");
        return -1;
    }

    is->width  = vout->width;
    is->height = vout->height;

    return 0;
}

/* display the current picture, if any */
static void video_display(FFPlayer *ffp)
{
    VideoState *is = &ffp->is;
    VideoPicture *vp = NULL;

    if (!is->video_st)
        return;

    vp = &is->pictq[is->pictq_rindex];
    if (vp && video_open(ffp, 0, vp))
        return;

    if (ffp->vout && vp)
        SDL_VoutRender(ffp->vout, vp->bmp);
}
#endif

