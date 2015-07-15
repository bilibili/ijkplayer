/*****************************************************************************
 * ijksdl_codec_android_mediadef.h
 *****************************************************************************
 *
 * copyright (c) 2014 Zhang Rui <bbcallen@gmail.com>
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

#ifndef IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIADEF_H
#define IJKSDL_ANDROID__ANDROID_CODEC_ANDROID_MEDIADEF_H

#include "../ijksdl_misc.h"

typedef enum sdl_amedia_status_t {
    SDL_AMEDIA_OK = 0,

    SDL_AMEDIA_ERROR_BASE                  = -10000,
    SDL_AMEDIA_ERROR_UNKNOWN               = SDL_AMEDIA_ERROR_BASE,
    SDL_AMEDIA_ERROR_MALFORMED             = SDL_AMEDIA_ERROR_BASE - 1,
    SDL_AMEDIA_ERROR_UNSUPPORTED           = SDL_AMEDIA_ERROR_BASE - 2,
    SDL_AMEDIA_ERROR_INVALID_OBJECT        = SDL_AMEDIA_ERROR_BASE - 3,
    SDL_AMEDIA_ERROR_INVALID_PARAMETER     = SDL_AMEDIA_ERROR_BASE - 4,

    SDL_AMEDIA_DRM_ERROR_BASE              = -20000,
    SDL_AMEDIA_DRM_NOT_PROVISIONED         = SDL_AMEDIA_DRM_ERROR_BASE - 1,
    SDL_AMEDIA_DRM_RESOURCE_BUSY           = SDL_AMEDIA_DRM_ERROR_BASE - 2,
    SDL_AMEDIA_DRM_DEVICE_REVOKED          = SDL_AMEDIA_DRM_ERROR_BASE - 3,
    SDL_AMEDIA_DRM_SHORT_BUFFER            = SDL_AMEDIA_DRM_ERROR_BASE - 4,
    SDL_AMEDIA_DRM_SESSION_NOT_OPENED      = SDL_AMEDIA_DRM_ERROR_BASE - 5,
    SDL_AMEDIA_DRM_TAMPER_DETECTED         = SDL_AMEDIA_DRM_ERROR_BASE - 6,
    SDL_AMEDIA_DRM_VERIFY_FAILED           = SDL_AMEDIA_DRM_ERROR_BASE - 7,
    SDL_AMEDIA_DRM_NEED_KEY                = SDL_AMEDIA_DRM_ERROR_BASE - 8,
    SDL_AMEDIA_DRM_LICENSE_EXPIRED         = SDL_AMEDIA_DRM_ERROR_BASE - 9,
} sdl_amedia_status_t;

#define SDL_AMIME_VIDEO_VP8         "video/x-vnd.on2.vp8"   //- VP8 video (i.e. video in .webm)
#define SDL_AMIME_VIDEO_VP9         "video/x-vnd.on2.vp9"   //- VP9 video (i.e. video in .webm)
#define SDL_AMIME_VIDEO_AVC         "video/avc"             //- H.264/AVC video
#define SDL_AMIME_VIDEO_HEVC        "video/hevc"            //- H.265/HEVC video
#define SDL_AMIME_VIDEO_MPEG4       "video/mp4v-es"         //- MPEG4 video
#define SDL_AMIME_VIDEO_H264        "video/3gpp"            //- H.263 video
#define SDL_AMIME_AUDIO_AMR_NB      "audio/3gpp"            //- AMR narrowband audio
#define SDL_AMIME_AUDIO_AMR_WB      "audio/amr-wb"          //- AMR wideband audio
#define SDL_AMIME_AUDIO_MP3         "audio/mpeg"            //- MPEG1/2 audio layer III
#define SDL_AMIME_AUDIO_RAW_AAC     "audio/mp4a-latm"       //- AAC audio (note, this is raw AAC packets, not packaged in LATM!)
#define SDL_AMIME_AUDIO_VORBIS      "audio/vorbis"          //- vorbis audio
#define SDL_AMIME_AUDIO_G711_ALAW   "audio/g711-alaw"       //- G.711 alaw audio
#define SDL_AMIME_AUDIO_G711_MLAW   "audio/g711-mlaw"       //- G.711 ulaw audio

#if 0
typedef enum sdl_amedia_format_key_t {
    AMEDIAFORMAT_KEY_AAC_PROFILE,
    AMEDIAFORMAT_KEY_BIT_RATE,
    AMEDIAFORMAT_KEY_CHANNEL_COUNT,
    AMEDIAFORMAT_KEY_CHANNEL_MASK,
    AMEDIAFORMAT_KEY_COLOR_FORMAT,
    AMEDIAFORMAT_KEY_DURATION,
    AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL,
    AMEDIAFORMAT_KEY_FRAME_RATE,
    AMEDIAFORMAT_KEY_HEIGHT,
    AMEDIAFORMAT_KEY_IS_ADTS,
    AMEDIAFORMAT_KEY_IS_AUTOSELECT,
    AMEDIAFORMAT_KEY_IS_DEFAULT,
    AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE,
    AMEDIAFORMAT_KEY_I_FRAME_INTERVAL,
    AMEDIAFORMAT_KEY_LANGUAGE,
    AMEDIAFORMAT_KEY_MAX_HEIGHT,
    AMEDIAFORMAT_KEY_MAX_INPUT_SIZE,
    AMEDIAFORMAT_KEY_MAX_WIDTH,
    AMEDIAFORMAT_KEY_MIME,
    AMEDIAFORMAT_KEY_PUSH_BLANK_BUFFERS_ON_STOP,
    AMEDIAFORMAT_KEY_REPEAT_PREVIOUS_FRAME_AFTER,
    AMEDIAFORMAT_KEY_SAMPLE_RATE,
    AMEDIAFORMAT_KEY_WIDTH,
    AMEDIAFORMAT_KEY_STRIDE,
} sdl_amedia_format_key_t;
#else

#define AMEDIAFORMAT_KEY_AAC_PROFILE            "aac-profile"
#define AMEDIAFORMAT_KEY_BIT_RATE               "bitrate"
#define AMEDIAFORMAT_KEY_CHANNEL_COUNT          "channel-count"
#define AMEDIAFORMAT_KEY_CHANNEL_MASK           "channel-mask"
#define AMEDIAFORMAT_KEY_COLOR_FORMAT           "color-format"
#define AMEDIAFORMAT_KEY_DURATION               "durationUs"
#define AMEDIAFORMAT_KEY_FLAC_COMPRESSION_LEVEL "flac-compression-level"
#define AMEDIAFORMAT_KEY_FRAME_RATE             "frame-rate"
#define AMEDIAFORMAT_KEY_HEIGHT                 "height"
#define AMEDIAFORMAT_KEY_IS_ADTS                "is-adts"
//#define AMEDIAFORMAT_KEY_IS_AUTOSELECT
//#define AMEDIAFORMAT_KEY_IS_DEFAULT
//#define AMEDIAFORMAT_KEY_IS_FORCED_SUBTITLE
#define AMEDIAFORMAT_KEY_I_FRAME_INTERVAL       "i-frame-interval"
//#define AMEDIAFORMAT_KEY_LANGUAGE
//#define AMEDIAFORMAT_KEY_MAX_HEIGHT
#define AMEDIAFORMAT_KEY_MAX_INPUT_SIZE         "max-input-size"
//#define AMEDIAFORMAT_KEY_MAX_WIDTH
#define AMEDIAFORMAT_KEY_MIME                   "mime"
//#define AMEDIAFORMAT_KEY_PUSH_BLANK_BUFFERS_ON_STOP,
//#define AMEDIAFORMAT_KEY_REPEAT_PREVIOUS_FRAME_AFTER,
#define AMEDIAFORMAT_KEY_SAMPLE_RATE            "sample-rate"
#define AMEDIAFORMAT_KEY_WIDTH                  "width"
//#define AMEDIAFORMAT_KEY_STRIDE
#endif

enum {
    AMEDIACODEC__INFO_OUTPUT_BUFFERS_CHANGED = -3,
    AMEDIACODEC__INFO_OUTPUT_FORMAT_CHANGED  = -2,
    AMEDIACODEC__INFO_TRY_AGAIN_LATER        = -1,

    AMEDIACODEC__UNKNOWN_ERROR               = -1000,
};

enum {
    AMEDIACODEC__BUFFER_FLAG_KEY_FRAME       = 0x01,
    AMEDIACODEC__BUFFER_FLAG_CODEC_CONFIG    = 0x02,
    AMEDIACODEC__BUFFER_FLAG_END_OF_STREAM   = 0x04,
};

enum {
    AMEDIACODEC__OMX_COLOR_FormatMonochrome              = 1,
    AMEDIACODEC__OMX_COLOR_Format8bitRGB332              = 2,
    AMEDIACODEC__OMX_COLOR_Format12bitRGB444             = 3,
    AMEDIACODEC__OMX_COLOR_Format16bitARGB4444           = 4,
    AMEDIACODEC__OMX_COLOR_Format16bitARGB1555           = 5,
    AMEDIACODEC__OMX_COLOR_Format16bitRGB565             = 6,
    AMEDIACODEC__OMX_COLOR_Format16bitBGR565             = 7,
    AMEDIACODEC__OMX_COLOR_Format18bitRGB666             = 8,
    AMEDIACODEC__OMX_COLOR_Format18bitARGB1665           = 9,
    AMEDIACODEC__OMX_COLOR_Format19bitARGB1666           = 10,
    AMEDIACODEC__OMX_COLOR_Format24bitRGB888             = 11,
    AMEDIACODEC__OMX_COLOR_Format24bitBGR888             = 12,
    AMEDIACODEC__OMX_COLOR_Format24bitARGB1887           = 13,
    AMEDIACODEC__OMX_COLOR_Format25bitARGB1888           = 14,
    AMEDIACODEC__OMX_COLOR_Format32bitBGRA8888           = 15,
    AMEDIACODEC__OMX_COLOR_Format32bitARGB8888           = 16,
    AMEDIACODEC__OMX_COLOR_FormatYUV411Planar            = 17,
    AMEDIACODEC__OMX_COLOR_FormatYUV411PackedPlanar      = 18,
    AMEDIACODEC__OMX_COLOR_FormatYUV420Planar            = 19,
    AMEDIACODEC__OMX_COLOR_FormatYUV420PackedPlanar      = 20,
    AMEDIACODEC__OMX_COLOR_FormatYUV420SemiPlanar        = 21,
    AMEDIACODEC__OMX_COLOR_FormatYUV422Planar            = 22,
    AMEDIACODEC__OMX_COLOR_FormatYUV422PackedPlanar      = 23,
    AMEDIACODEC__OMX_COLOR_FormatYUV422SemiPlanar        = 24,
    AMEDIACODEC__OMX_COLOR_FormatYCbYCr                  = 25,
    AMEDIACODEC__OMX_COLOR_FormatYCrYCb                  = 26,
    AMEDIACODEC__OMX_COLOR_FormatCbYCrY                  = 27,
    AMEDIACODEC__OMX_COLOR_FormatCrYCbY                  = 28,
    AMEDIACODEC__OMX_COLOR_FormatYUV444Interleaved       = 29,
    AMEDIACODEC__OMX_COLOR_FormatRawBayer8bit            = 30,
    AMEDIACODEC__OMX_COLOR_FormatRawBayer10bit           = 31,
    AMEDIACODEC__OMX_COLOR_FormatRawBayer8bitcompressed  = 32,
    AMEDIACODEC__OMX_COLOR_FormatL2                      = 33,
    AMEDIACODEC__OMX_COLOR_FormatL4                      = 34,
    AMEDIACODEC__OMX_COLOR_FormatL8                      = 35,
    AMEDIACODEC__OMX_COLOR_FormatL16                     = 36,
    AMEDIACODEC__OMX_COLOR_FormatL24                     = 37,
    AMEDIACODEC__OMX_COLOR_FormatL32                     = 38,
    AMEDIACODEC__OMX_COLOR_FormatYUV420PackedSemiPlanar  = 39,
    AMEDIACODEC__OMX_COLOR_FormatYUV422PackedSemiPlanar  = 40,
    AMEDIACODEC__OMX_COLOR_Format18BitBGR666             = 41,
    AMEDIACODEC__OMX_COLOR_Format24BitARGB6666           = 42,
    AMEDIACODEC__OMX_COLOR_Format24BitABGR6666           = 43,

    // AMEDIACODEC__OMX_COLOR_TI_FormatYUV420PackedSemiPlanar = 0x7f000100,
    AMEDIACODEC__OMX_COLOR_FormatSurface                   = 0x7f000789,
    AMEDIACODEC__OMX_COLOR_FormatYUV420Flexible            = 0x7f420888,
    // AMEDIACODEC__OMX_COLOR_QCOM_FormatYUV420SemiPlanar     = 0x7fa30c00,

    // from hardware intel
    _AMEDIACODEC__OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar              = 0x7FA00E00,
    _AMEDIACODEC__OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar_Tiled        = 0x7FA00F00,

    // from hardware qcom
    _AMEDIACODEC__OMX_QCOM_COLOR_FormatYVU420SemiPlanar                     = 0x7FA30C00,
    _AMEDIACODEC__OMX_QCOM_COLOR_FormatYVU420PackedSemiPlanar32m4ka         = 0x7FA30C01,
    _AMEDIACODEC__OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar16m2ka         = 0x7FA30C02,
    _AMEDIACODEC__OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar64x32Tile2m8ka = 0x7FA30C03,
    _AMEDIACODEC__OMX_QCOM_COLOR_FORMATYUV420PackedSemiPlanar32m            = 0x7FA30C04,
    _AMEDIACODEC__OMX_QCOM_COLOR_FORMATYUV420PackedSemiPlanar32mMultiView   = 0x7FA30C05,

    // from hardware samsung_slsi
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12TPhysicalAddress  = 0x7F000001,
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12LPhysicalAddress  = 0x7F000002,
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12LVirtualAddress   = 0x7F000003,
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12Tiled             = 0x7FC00002,
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV21LPhysicalAddress  = 0x7F000010,
    _AMEDIACODEC__OMX_SEC_COLOR_FormatNV21Linear            = 0x7F000011,

    // from hardware ti
    // _AMEDIACODEC__OMX_TI_COLOR_FormatRawBayer10bitStereo    = 0x7F000002,
    _AMEDIACODEC__OMX_TI_COLOR_FormatYUV420PackedSemiPlanar = 0x7F000100,
};

const char *SDL_AMediaCodec_getColorFormatName(int colorFormat);

//#define AMCTRACE ALOGI
#define AMCTRACE(...)

#endif

