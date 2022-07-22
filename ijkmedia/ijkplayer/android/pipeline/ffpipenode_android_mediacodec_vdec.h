/*
 * ffpipenode_android_mediacodec_vdec.h
 *
 * Copyright (c) 2014 Bilibili
 * Copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef FFPLAY__FF_FFPIPENODE_ANDROID_MEDIACODEC_VDEC_H
#define FFPLAY__FF_FFPIPENODE_ANDROID_MEDIACODEC_VDEC_H

#include "../../ff_ffpipenode.h"
#include "../../ff_ffpipeline.h"
#include "ijksdl/ijksdl_vout.h"

typedef struct FFPlayer FFPlayer;

IJKFF_Pipenode *ffpipenode_create_video_decoder_from_android_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout);
IJKFF_Pipenode *ffpipenode_init_decoder_from_android_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout);
int ffpipenode_config_from_android_mediacodec(FFPlayer *ffp, IJKFF_Pipeline *pipeline, SDL_Vout *vout, IJKFF_Pipenode *node);

#endif
