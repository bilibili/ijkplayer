/*****************************************************************************
 * ijkadk_android_media_MediaCodec_BufferInfo.hpp
 *****************************************************************************
 *
 * copyright (c) 2013-2014 Zhang Rui <bbcallen@gmail.com>
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
 
#ifndef IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_BUFFERINFO_HPP
#define IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_BUFFERINFO_HPP

#include "ijkadkobject.hpp"
#include "ijkadk_java_lang.hpp"
#include "ijkadk_java_nio.hpp"
#include <vector>

namespace ijkadk {
namespace android {
namespace media {
namespace MediaCodec_ {

using namespace ::std;
using namespace ::ijkadk::java::lang;
using namespace ::ijkadk::java::nio;

// API-16
class BufferInfo: public ::ijkadk::ADKObject
{
public:
    ADK_OBJ_BEGIN(BufferInfo);
    ADK_OBJ_END();

public:

public:
    static int loadClass(JNIEnv *env);
};

} // end ::ijkadk::android::media::MediaCodec_
} // end ::ijkadk::android::media
} // end ::ijkadk::android
} // end ::ijkadk

#endif /* IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_BUFFERINFO_HPP */
