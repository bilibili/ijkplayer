/*****************************************************************************
 * IJKVideoToolBox.h
 *****************************************************************************
 *
 * copyright (c) 2014 Zhou Quan <zhouqicy@gmail.com>
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

#ifndef IJKMediaPlayer_videotoolbox_core_h
#define IJKMediaPlayer_videotoolbox_core_h


#import <VideoToolbox/VideoToolbox.h>
#include "ff_ffinc.h"
#include "ff_fferror.h"
#include "ff_ffmsg.h"
#include "ff_ffplay.h"
#include "ijksdl/ios/ijksdl_vout_overlay_videotoolbox.h"


#define MAX_PKT_QUEUE_DEEP   350
#define VTB_MAX_DECODING_SAMPLES 3

typedef struct VideoToolBoxContext VideoToolBoxContext;

VideoToolBoxContext* videotoolbox_create(FFPlayer* ffp, AVCodecContext* ic);

int videotoolbox_decode_frame(VideoToolBoxContext* context);

void videotoolbox_free(VideoToolBoxContext* context);

#endif
