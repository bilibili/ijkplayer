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

#include "ijksdl_codec_android_mediadef.h"

const char *SDL_AMediaCodec_getColorFormatName(int colorFormat)
{
    switch (colorFormat) {
    case AMEDIACODEC__OMX_COLOR_FormatMonochrome:
        return "Format8bitRGB332";
    case AMEDIACODEC__OMX_COLOR_Format8bitRGB332:
        return "Format8bitRGB332";
    case AMEDIACODEC__OMX_COLOR_Format12bitRGB444:
        return "Format12bitRGB444";
    case AMEDIACODEC__OMX_COLOR_Format16bitARGB4444:
        return "Format16bitARGB4444";
    case AMEDIACODEC__OMX_COLOR_Format16bitARGB1555:
        return "Format16bitARGB1555";
    case AMEDIACODEC__OMX_COLOR_Format16bitRGB565:
        return "Format16bitRGB565";
    case AMEDIACODEC__OMX_COLOR_Format16bitBGR565:
        return "Format16bitBGR565";
    case AMEDIACODEC__OMX_COLOR_Format18bitRGB666:
        return "Format18bitRGB666";
    case AMEDIACODEC__OMX_COLOR_Format18bitARGB1665:
        return "Format18bitARGB1665";
    case AMEDIACODEC__OMX_COLOR_Format19bitARGB1666:
        return "Format19bitARGB1666";
    case AMEDIACODEC__OMX_COLOR_Format24bitRGB888:
        return "Format24bitRGB888";
    case AMEDIACODEC__OMX_COLOR_Format24bitBGR888:
        return "Format24bitBGR888";
    case AMEDIACODEC__OMX_COLOR_Format24bitARGB1887:
        return "Format24bitARGB1887";
    case AMEDIACODEC__OMX_COLOR_Format25bitARGB1888:
        return "Format25bitARGB1888";
    case AMEDIACODEC__OMX_COLOR_Format32bitBGRA8888:
        return "Format32bitBGRA8888";
    case AMEDIACODEC__OMX_COLOR_Format32bitARGB8888:
        return "Format32bitARGB8888";
    case AMEDIACODEC__OMX_COLOR_FormatYUV411Planar:
        return "FormatYUV411Planar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV411PackedPlanar:
        return "FormatYUV411PackedPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV420Planar:
        return "FormatYUV420Planar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV420PackedPlanar:
        return "FormatYUV420PackedPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV420SemiPlanar:
        return "FormatYUV420SemiPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV422Planar:
        return "FormatYUV422Planar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV422PackedPlanar:
        return "FormatYUV422PackedPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV422SemiPlanar:
        return "FormatYUV422SemiPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYCbYCr:
        return "FormatYCbYCr";
    case AMEDIACODEC__OMX_COLOR_FormatYCrYCb:
        return "FormatYCrYCb";
    case AMEDIACODEC__OMX_COLOR_FormatCbYCrY:
        return "FormatCbYCrY";
    case AMEDIACODEC__OMX_COLOR_FormatCrYCbY:
        return "FormatCrYCbY";
    case AMEDIACODEC__OMX_COLOR_FormatYUV444Interleaved:
        return "FormatYUV444Interleaved";
    case AMEDIACODEC__OMX_COLOR_FormatRawBayer8bit:
        return "FormatRawBayer8bit";
    case AMEDIACODEC__OMX_COLOR_FormatRawBayer10bit:
        return "FormatRawBayer10bit";
    case AMEDIACODEC__OMX_COLOR_FormatRawBayer8bitcompressed:
        return "FormatRawBayer8bitcompressed";
    case AMEDIACODEC__OMX_COLOR_FormatL2:
        return "FormatL2";
    case AMEDIACODEC__OMX_COLOR_FormatL4:
        return "FormatL4";
    case AMEDIACODEC__OMX_COLOR_FormatL8:
        return "FormatL8";
    case AMEDIACODEC__OMX_COLOR_FormatL16:
        return "FormatL16";
    case AMEDIACODEC__OMX_COLOR_FormatL24:
        return "FormatL24";
    case AMEDIACODEC__OMX_COLOR_FormatL32:
        return "FormatL32";
    case AMEDIACODEC__OMX_COLOR_FormatYUV420PackedSemiPlanar:
        return "FormatYUV420PackedSemiPlanar";
    case AMEDIACODEC__OMX_COLOR_FormatYUV422PackedSemiPlanar:
        return "FormatYUV422PackedSemiPlanar";
    case AMEDIACODEC__OMX_COLOR_Format18BitBGR666:
        return "Format18BitBGR666";
    case AMEDIACODEC__OMX_COLOR_Format24BitARGB6666:
        return "Format24BitARGB6666";
    case AMEDIACODEC__OMX_COLOR_Format24BitABGR6666:
        return "Format24BitABGR6666";

#if 0
        // duplicated
    case AMEDIACODEC__OMX_COLOR_TI_FormatYUV420PackedSemiPlanar:
        return "TI_FormatYUV420PackedSemiPlanar";
#endif
    case AMEDIACODEC__OMX_COLOR_FormatSurface:
        return "FormatSurface";
    case AMEDIACODEC__OMX_COLOR_FormatYUV420Flexible:
        return "FormatYUV420Flexible";
#if 0
        // duplicated
    case AMEDIACODEC__OMX_COLOR_QCOM_FormatYUV420SemiPlanar:
        return "QCOM_FormatYUV420SemiPlanar";
#endif

        // from hardware intel
    case _AMEDIACODEC__OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar:
        return "INTEL_FormatYUV420PackedSemiPlanar";
    case _AMEDIACODEC__OMX_INTEL_COLOR_FormatYUV420PackedSemiPlanar_Tiled:
        return "INTEL_FormatYUV420PackedSemiPlanar_Tiled";

        // from hardware qcom
    case _AMEDIACODEC__OMX_QCOM_COLOR_FormatYVU420SemiPlanar:
        return "QCOM_FormatYVU420SemiPlanar";
    case _AMEDIACODEC__OMX_QCOM_COLOR_FormatYVU420PackedSemiPlanar32m4ka:
        return "QCOM_FormatYVU420PackedSemiPlanar32m4ka";
    case _AMEDIACODEC__OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar16m2ka:
        return "QCOM_FormatYUV420PackedSemiPlanar16m2ka";
    case _AMEDIACODEC__OMX_QCOM_COLOR_FormatYUV420PackedSemiPlanar64x32Tile2m8ka:
        return "QCOM_FormatYUV420PackedSemiPlanar64x32Tile2m8ka";
    case _AMEDIACODEC__OMX_QCOM_COLOR_FORMATYUV420PackedSemiPlanar32m:
        return "QCOM_FORMATYUV420PackedSemiPlanar32m";
    case _AMEDIACODEC__OMX_QCOM_COLOR_FORMATYUV420PackedSemiPlanar32mMultiView:
        return "QCOM_FORMATYUV420PackedSemiPlanar32mMultiView";

        // from hardware samsung_slsi
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12TPhysicalAddress:
        return "SEC_FormatNV12TPhysicalAddress";
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12LPhysicalAddress:
        return "SEC_FormatNV12LPhysicalAddress";
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12LVirtualAddress:
        return "SEC_FormatNV12LVirtualAddress";
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV12Tiled:
        return "SEC_FormatNV12Tiled";
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV21LPhysicalAddress:
        return "SEC_FormatNV21LPhysicalAddress";
    case _AMEDIACODEC__OMX_SEC_COLOR_FormatNV21Linear:
        return "SEC_FormatNV21Linear";

        // from hardware ti
    case _AMEDIACODEC__OMX_TI_COLOR_FormatYUV420PackedSemiPlanar:
        return "TI_FormatYUV420PackedSemiPlanar";

    default:
        return "FormatUnknown";
    }
}
