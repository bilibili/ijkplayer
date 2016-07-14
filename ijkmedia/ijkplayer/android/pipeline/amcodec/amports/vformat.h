/**
* @file vformat.h
* @brief  Porting from decoder driver for video format
* @author Tim Yao <timyao@amlogic.com>
* @version 1.0.0
* @date 2011-02-24
*/
/* Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
* 
*/

/*
 * AMLOGIC Audio/Video streaming port driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Author:  Tim Yao <timyao@amlogic.com>
 *
 */

#ifndef VFORMAT_H
#define VFORMAT_H

typedef enum {
    VIDEO_DEC_FORMAT_UNKNOW,
    VIDEO_DEC_FORMAT_MPEG4_3,
    VIDEO_DEC_FORMAT_MPEG4_4,
    VIDEO_DEC_FORMAT_MPEG4_5,
    VIDEO_DEC_FORMAT_H264,
    VIDEO_DEC_FORMAT_MJPEG,
    VIDEO_DEC_FORMAT_MP4,
    VIDEO_DEC_FORMAT_H263,
    VIDEO_DEC_FORMAT_REAL_8,
    VIDEO_DEC_FORMAT_REAL_9,
    VIDEO_DEC_FORMAT_WMV3,
    VIDEO_DEC_FORMAT_WVC1,
    VIDEO_DEC_FORMAT_SW,
    VIDEO_DEC_FORMAT_AVS,
    VIDEO_DEC_FORMAT_H264_4K2K,
    VIDEO_DEC_FORMAT_HEVC,
    VIDEO_DEC_FORMAT_MAX
} vdec_type_t;

typedef enum {
    VFORMAT_UNKNOWN = -1,
    VFORMAT_MPEG12 = 0,
    VFORMAT_MPEG4,
    VFORMAT_H264,
    VFORMAT_MJPEG,
    VFORMAT_REAL,
    VFORMAT_JPEG,
    VFORMAT_VC1,
    VFORMAT_AVS,
    VFORMAT_SW,
    VFORMAT_H264MVC,
    VFORMAT_H264_4K2K,
    VFORMAT_HEVC,
    VFORMAT_UNSUPPORT,
    VFORMAT_MAX
} vformat_t;

#define IS_VFMT_VALID(vfmt)	((vfmt > VFORMAT_UNKNOWN) && (vfmt < VFORMAT_MAX))
#define IS_NEED_VDEC_INFO(vfmt) ((vfmt == VFORMAT_MPEG4) || (vfmt == VFORMAT_REAL))

#define CODEC_TAG_MJPEG     (0x47504a4d)
#define CODEC_TAG_mjpeg     (0x47504a4c)
#define CODEC_TAG_jpeg      (0x6765706a)
#define CODEC_TAG_mjpa      (0x61706a6d)
#define CODEC_TAG_XVID      (0x44495658)
#define CODEC_TAG_xvid      (0x64697678)
#define CODEC_TAG_XVIX      (0x58495658)
#define CODEC_TAG_xvix      (0x78697678)
#define CODEC_TAG_MP4       (0x8e22ada)
#define CODEC_TAG_COL1      (0x314c4f43)
#define CODEC_TAG_DIV3      (0x33564944)
#define CODEC_TAG_MP43      (0x3334504d)
#define CODEC_TAG_M4S2      (0x3253344d)
#define CODEC_TAG_DIV4      (0x34564944)
#define CODEC_TAG_DIVX      (0x58564944)
#define CODEC_TAG_DIV5      (0x35564944)
#define CODEC_TAG_DX50      (0x30355844)
#define CODEC_TAG_DIV6      (0x36564944)
#define CODEC_TAG_RMP4      (0x34504d52)
#define CODEC_TAG_MP42      (0x3234504d)
#define CODEC_TAG_MPG4      (0x3447504d)
#define CODEC_TAG_MP4V      (0x5634504d)
#define CODEC_TAG_mp4v      (0x7634706d)
#define CODEC_TAG_AVC1      (0x31435641)
#define CODEC_TAG_avc1      (0x31637661)
#define CODEC_TAG_H264      (0x34363248)
#define CODEC_TAG_h264      (0x34363268)
#define CODEC_TAG_HEVC      (0x43564548)
#define CODEC_TAG_hvc1      (0x31637668)
#define CODEC_TAG_hev1      (0x31766568)
#define CODEC_TAG_H263      (0x33363248)
#define CODEC_TAG_h263      (0x33363268)
#define CODEC_TAG_s263      (0x33363273)
#define CODEC_TAG_F263      (0x33363246)
#define CODEC_TAG_WMV1      (0x31564d57)
#define CODEC_TAG_WMV2      (0x32564d57)
#define CODEC_TAG_WMV3      (0x33564d57)
#define CODEC_TAG_WVC1      (0x31435657)
#define CODEC_TAG_WMVA      (0x41564d57)
#define CODEC_TAG_FMP4      (0x34504d46)

#endif /* VFORMAT_H */
