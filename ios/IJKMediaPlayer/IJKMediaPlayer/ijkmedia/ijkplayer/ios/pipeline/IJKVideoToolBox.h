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


typedef struct sample_info {
    int     sample_id;

    double  sort;
    double  dts;
    double  pts;
    int     serial;

    int     sar_num;
    int     sar_den;

    volatile int is_decoding;
} sample_info;


typedef struct sort_queue {
    VTBPicture pic;
    int serial;
    volatile struct sort_queue  *nextframe;
} sort_queue;


typedef struct VideoToolBoxContext {
    FFPlayer                   *ffp;
    int                         width;
    int                         height;
    volatile bool               refresh_request;
    volatile bool               new_seg_flag;
    volatile bool               idr_based_identified;
    int64_t                     last_keyframe_pts;
    volatile bool               refresh_session;
    volatile bool               recovery_drop_packet;
    VTDecompressionSessionRef   m_vt_session;
    CMFormatDescriptionRef      m_fmt_desc;
    const char                 *m_pformat_name;
    struct VTBPicture           m_videobuffer;
    double                      m_sort_time_offset;
    pthread_mutex_t             m_queue_mutex;
    volatile sort_queue        *m_sort_queue;
    volatile int32_t            m_queue_depth;
    int32_t                     m_max_ref_frames;
    bool                        m_convert_bytestream;
    bool                        m_convert_3byteTo4byteNALSize;
    int                         serial;
    volatile double             last_sort;
    bool                        dealloced;
    int                         m_buffer_deep;
    AVPacket                    m_buffer_packet[MAX_PKT_QUEUE_DEEP];

    SDL_mutex                  *sample_info_mutex;
    SDL_cond                   *sample_info_cond;
    sample_info                 sample_info_array[VTB_MAX_DECODING_SAMPLES];
    volatile int                sample_info_index;
    volatile int                sample_info_id_generator;
    volatile int                sample_infos_in_decoding;

    Uint64                      benchmark_start_time;
    Uint64                      benchmark_frame_count;
} VideoToolBoxContext ;


VideoToolBoxContext* init_videotoolbox(FFPlayer* ffp, AVCodecContext* ic);

int videotoolbox_decode_video(VideoToolBoxContext* context, AVCodecContext *avctx, const AVPacket *avpkt,int* got_picture_ptr);

void dealloc_videotoolbox(VideoToolBoxContext* context);

#endif
