/*****************************************************************************
 * yuv_rgb.c : ARM NEONv1 YUV to RGB32 chroma conversion for VLC
 *****************************************************************************
 * Copyright (C) 2011 Sébastien Toque
 *                    Rémi Denis-Courmont
 * Copyright (C) 2013 Zhang Rui <bbcallen@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "../ijksdl_image_convert.h"

#include <cpu-features.h>
#include "chroma_neon.h"

static void ijk_i420_rgb32_neon(int width, int height, uint8_t **dst_data, int *dst_linesize, const uint8_t **src_data, int *src_linesize)
{
    struct yuv_pack out = { dst_data[0], dst_linesize[0] };
    struct yuv_planes_in in = { src_data[0], src_data[1], src_data[2], src_linesize[0] };
    i420_rgb_neon(&out, &in, width, height);
}

static void ijk_i420_rgb16_neon(int width, int height, uint8_t **dst_data, int *dst_linesize, const uint8_t **src_data, int *src_linesize)
{
    struct yuv_pack out = { dst_data[0], dst_linesize[0] };
    struct yuv_planes_in in = { src_data[0], src_data[1], src_data[2], src_linesize[0] };
    i420_rv16_neon(&out, &in, width, height);
}

// TODO: 9 need nv12 and nv21 test sample
#if 0
static void ijk_nv21_rgb32_neon(int width, int height, uint8_t **dst_data, int *dst_linesize, const uint8_t **src_data, int *src_linesize)
{
    struct yuv_pack out = {dst_data[0], dst_linesize[0]};
    struct yuv_planes_in in = {src_data[0], src_data[1], src_data[2], src_linesize[0]};
    nv21_rgb_neon(&out, &in, width, height);
}

static void ijk_nv12_rgb32_neon(int width, int height, uint8_t **dst_data, int *dst_linesize, const uint8_t **src_data, int *src_linesize)
{
    struct yuv_pack out = {dst_data[0], dst_linesize[0]};
    struct yuv_planes_in in = {src_data[0], src_data[1], src_data[2], src_linesize[0]};
    nv12_rgb_neon(&out, &in, width, height);
}
#endif

int ijk_image_convert(int width, int height,
    enum AVPixelFormat dst_format, uint8_t **dst_data, int *dst_linesize,
    enum AVPixelFormat src_format, const uint8_t **src_data, int *src_linesize)
{
    if (!(android_getCpuFeatures() & (ANDROID_CPU_ARM_FEATURE_ARMv7 | ANDROID_CPU_ARM_FEATURE_NEON)))
        return -1;

    switch (src_format) {
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P: // FIXME: 9 not equal to AV_PIX_FMT_YUV420P, but a workaround
        switch (dst_format) {
        case AV_PIX_FMT_RGB565:
            ijk_i420_rgb16_neon(width, height, dst_data, dst_linesize, src_data, src_linesize);
            return 0;
            break;
        case AV_PIX_FMT_0BGR32:
            ijk_i420_rgb32_neon(width, height, dst_data, dst_linesize, src_data, src_linesize);
            return 0;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return -1;
}

