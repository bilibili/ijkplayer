/* SPDX-License-Identifier: LGPL-2.1-or-later */
#ifndef IJKPLAYER_ANDROID_FFAMC_OUTPUT_SIZE_H
#define IJKPLAYER_ANDROID_FFAMC_OUTPUT_SIZE_H
#include <strings.h>

/* Included after the pipeline state; shared by both output dequeue modes.
 * MediaCodec may change only its visible crop, without NEW_EXTRADATA or a
 * codec reconfiguration. Publish that geometry through the following frames. */
static void update_output_frame_size(IJKFF_Pipenode_Opaque *opaque,
                                     int width, int height, int crop_mask,
                                     int left, int top, int right, int bottom)
{
    /* Keep other codecs and the historical TI H.264 workaround unchanged. */
    if (opaque->codecpar->codec_id != AV_CODEC_ID_H264 ||
        !strncasecmp(opaque->acodec_name, "OMX.TI.DUCATI1.", 15))
        return;
    /* Partial/invalid vendor geometry must not break existing playback. */
    if (width <= 0 || height <= 0 || (crop_mask != 0 && crop_mask != 15))
        return;
    if (crop_mask) {
        if (left < 0 || top < 0 || right < left || bottom < top ||
            right >= width || bottom >= height)
            return;
        width = right - left + 1;
        height = bottom - top + 1;
    }
    if (opaque->frame_rotate_degrees == 90 || opaque->frame_rotate_degrees == 270) {
        int swap = width; width = height; height = swap;
    }
    opaque->frame_width = width;
    opaque->frame_height = height;
}
#endif
