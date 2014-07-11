/*****************************************************************************
 * ijkadk_android_media_MediaCodec.cpp
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

#include "ijkadk_android_media_MediaCodec.hpp"

#include "ijkadk_java_lang_String.hpp"
#include "ijkadkutils.h"

using namespace ::ijkadk;
using namespace ::ijkadk::android::media;
using namespace ::ijkadk::java::lang;

typedef struct MediaCodecClass
{
    jclass      clazz;

    jmethodID   createByCodecName;
    jmethodID   createDecoderByType;
} MediaCodecClass;
static MediaCodecClass gClazz;

int MediaCodec::loadClass(JNIEnv *env)
{
    IJKADK_FIND_CLASS(env, gClazz.clazz, "Landroid/media/MediaCodec;");

    IJKADK_FIND_METHOD(env, gClazz.createByCodecName, gClazz.clazz,
        "createByCodecName",        "(Ljava/lang/String;)Landroid/media/MediaCodec;");
    IJKADK_FIND_METHOD(env, gClazz.createDecoderByType, gClazz.clazz,
        "createDecoderByType",      "(Ljava/lang/String;)Landroid/media/MediaCodec;");

    return 0;
}

ADKPtr<MediaCodec> MediaCodec::createByCodecName(const ADKPtr<String>& name)
{
    JNIEnv *env = ijkadk_get_env();

    ADKLocalRef<jobject> ret(env->CallStaticObjectMethod(gClazz.clazz, gClazz.createByCodecName, name.get()));
    IJKADK_CHECK_EXCEPTION();

    return MediaCodec::create(ret.get());
}

ADKPtr<MediaCodec> MediaCodec::createDecoderByType(const ADKPtr<String>& type)
{
    JNIEnv *env = ijkadk_get_env();

    ADKLocalRef<jobject> ret(env->CallStaticObjectMethod(gClazz.clazz, gClazz.createDecoderByType, type.get()));
    IJKADK_CHECK_EXCEPTION();

    return MediaCodec::create(ret.get());
}
