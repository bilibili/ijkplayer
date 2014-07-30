/*****************************************************************************
 * ijkadk_android_media_MediaCodec.hpp
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
 
#ifndef IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_HPP
#define IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_HPP

#include "ijkadkobject.hpp"
#include "ijkadk_java_lang.hpp"
#include "ijkadk_java_nio.hpp"
#include <vector>

namespace ijkadk {
namespace android {
namespace media {

using namespace ::std;
using namespace ::ijkadk::java::lang;
using namespace ::ijkadk::java::nio;

// API-16
class MediaCodec: public ::ijkadk::ADKObject
{
public:
    ADK_OBJ_BEGIN(MediaCodec);
    ADK_OBJ_END();

public:
    static ADKPtr<MediaCodec> createByCodecName(const ADKPtr<String>& name);
    static ADKPtr<MediaCodec> createByCodecName(const char *name) {return createByCodecName(ADKString(name));}
    static ADKPtr<MediaCodec> createDecoderByType(const ADKPtr<String>& type);
    static ADKPtr<MediaCodec> createDecoderByType(const char *type) {return createDecoderByType(ADKString(type));}

    // void configure(MediaFormat format, Surface surface, MediaCrypto crypto, jint flags);

    jint dequeueInputBuffer(jlong timeoutUs);
    // jint dequeueOutputBuffer(MediaCodec.BufferInfo info, jlong timeoutUs);

    void flush();

    vector<ADKPtr<ByteBuffer> > getInputBuffers();
    vector<ADKPtr<ByteBuffer> > getOutputBuffers();

    void queueInputBuffer(jint index, jint offset, jint size, jlong presentationTimeUs, jint flags);
    void release();
    void releaseOutputBuffer_render(int index, jboolean render);
    void releaseOutputBuffer_timestamp(int index, jlong renderTimestampNs);

    void start();
    void stop();

    // API-18
    // ADKPtr<String>         getName ()
    // ADKPtr<MediaCodecInfo> getCodecInfo();

public:
    static int loadClass(JNIEnv *env);
};

} // end ::ijkadk::android::media
} // end ::ijkadk::android
} // end ::ijkadk

#endif /* IJKADK__IJKADK_ANDROID_MEDIA_MEDIACODEC_HPP */
