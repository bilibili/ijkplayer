/*
 * ffpipenode_ios_videotoolbox_vdec.h
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

#ifndef FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_DEC_H
#define FFPLAY__FF_FFPIPENODE_IOS_VIDEOTOOLBOX_DEC_H

#include "ijkplayer/ff_ffpipenode.h"

struct FFPlayer;

IJKFF_Pipenode *ffpipenode_create_video_decoder_from_ios_videotoolbox(struct FFPlayer *ffp);

#endif
