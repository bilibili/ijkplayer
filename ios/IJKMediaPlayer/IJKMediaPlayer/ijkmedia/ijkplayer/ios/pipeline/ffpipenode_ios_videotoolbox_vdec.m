/*
 * ffpipenode_ios_videotoolbox_vdec.m
 *
 * Copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
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

#include "ffpipenode_ios_videotoolbox_vdec.h"
#include "IJKVideoToolBox.h"
#include "ijksdl_vout_overlay_videotoolbox.h"
#include "ijkplayer/ff_ffpipeline.h"
#include "ijkplayer/ff_ffpipenode.h"
#include "ijkplayer/ff_ffplay.h"
#include "ijksdl_mutex.h"
#include "ijksdl_vout_ios_gles2.h"
#import <UIKit/UIKit.h>

struct IJKFF_Pipenode_Opaque {
    IJKFF_Pipeline           *pipeline;
    FFPlayer                 *ffp;
    Decoder                  *decoder;
    VideoToolBoxContext      *context;
    AVCodecContext           *avctx; // not own
    SDL_Thread*              video_fill_thread;
    SDL_Thread              _video_fill_thread;
};

int videotoolbox_video_thread(void *arg)
{
    IJKFF_Pipenode_Opaque* opaque = (IJKFF_Pipenode_Opaque*) arg;
    FFPlayer *ffp = opaque->ffp;
    VideoState *is = ffp->is;
    Decoder   *d = &is->viddec;
    int ret = 0;

    for (;;) {

        if (is->abort_request || d->queue->abort_request) {
            return -1;
        }
        @autoreleasepool {
            ret = videotoolbox_decode_frame(opaque->context);
        }
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;

        if (ret < 0)
            goto the_end;
    }
the_end:
    return 0;
}



static void func_destroy(IJKFF_Pipenode *node)
{
    // do nothing
}

static int func_run_sync(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    int ret = videotoolbox_video_thread(opaque);

    if (opaque->context) {
        videotoolbox_free(opaque->context);
        free(opaque->context);
        opaque->context = NULL;
    }

    return ret;
}



IJKFF_Pipenode *ffpipenode_create_video_decoder_from_ios_videotoolbox(FFPlayer *ffp)
{
    if (!ffp || !ffp->is)
        return NULL;
    if ([[[UIDevice currentDevice] systemVersion] floatValue]  < 8.0){
        return NULL;
    }
    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKFF_Pipenode_Opaque));
    if (!node)
        return node;
    memset(node, sizeof(IJKFF_Pipenode), 0);

    VideoState            *is         = ffp->is;
    IJKFF_Pipenode_Opaque *opaque     = node->opaque;
    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    opaque->ffp         = ffp;
    opaque->decoder     = &is->viddec;
    opaque->avctx = opaque->decoder->avctx;
    switch (opaque->avctx->codec_id) {
    case AV_CODEC_ID_H264:
        opaque->context = videotoolbox_create(ffp, opaque->avctx);
        break;
    default:
        ALOGI("Videotoolbox-pipeline:open_video_decoder: not H264\n");
        goto fail;
    }
    if (opaque->context == NULL) {
        ALOGE("could not init video tool box decoder !!!");
        goto fail;
    }
    return node;

fail:
    ffpipenode_free_p(&node);
    return NULL;
}
