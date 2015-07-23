/*
 * ffpipenode_ios_videotoolbox_vout.m
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

#include "ffpipenode_ios_videotoolbox_vout.h"
#include "ff_ffpipeline.h"
#include "IJKVideoToolBox.h"
#include "ijkplayer/ff_ffpipenode.h"
#include "ijkplayer/ff_ffplay.h"

struct IJKFF_Pipenode_Opaque {
    FFPlayer *ffp;
};

static void func_destroy(IJKFF_Pipenode *node)
{
    // do nothing
}

static int func_run_sync(IJKFF_Pipenode *node)
{
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    int ret = ffp_video_refresh_thread(opaque->ffp);
    return ret;
}

IJKFF_Pipenode *ffpipenode_create_video_output_from_ios_videotoolbox(FFPlayer *ffp)
{
    IJKFF_Pipenode *node = ffpipenode_alloc(sizeof(IJKFF_Pipenode_Opaque));
    if (!node)
        return node;
    IJKFF_Pipenode_Opaque *opaque = node->opaque;
    opaque->ffp         = ffp;
    node->func_destroy  = func_destroy;
    node->func_run_sync = func_run_sync;
    return node;
}
