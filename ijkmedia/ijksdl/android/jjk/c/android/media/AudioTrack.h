/*
 * copyright (c) 2015 Zhang Rui <bbcallen@gmail.com>
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

#ifndef JJK__android_media_AudioTrack__H
#define JJK__android_media_AudioTrack__H

#include "ijksdl/android/jjk/internal/jjk_internal.h"

jobject JJKC_AudioTrack__AudioTrack(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode);
jobject JJKC_AudioTrack__AudioTrack__catchAll(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode);
jobject JJKC_AudioTrack__AudioTrack__asGlobalRef__catchAll(JNIEnv *env, jint streamType, jint sampleRateInHz, jint channelConfig, jint audioFormat, jint bufferSizeInBytes, jint mode);
jint JJKC_AudioTrack__getMinBufferSize(JNIEnv *env, jint sampleRateInHz, jint channelConfig, jint audioFormat);
jint JJKC_AudioTrack__getMinBufferSize__catchAll(JNIEnv *env, jint sampleRateInHz, jint channelConfig, jint audioFormat);
jfloat JJKC_AudioTrack__getMaxVolume(JNIEnv *env);
jfloat JJKC_AudioTrack__getMaxVolume__catchAll(JNIEnv *env);
jfloat JJKC_AudioTrack__getMinVolume(JNIEnv *env);
jfloat JJKC_AudioTrack__getMinVolume__catchAll(JNIEnv *env);
jint JJKC_AudioTrack__getNativeOutputSampleRate(JNIEnv *env, jint streamType);
jint JJKC_AudioTrack__getNativeOutputSampleRate__catchAll(JNIEnv *env, jint streamType);
void JJKC_AudioTrack__play(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__play__catchAll(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__pause(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__pause__catchAll(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__stop(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__stop__catchAll(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__flush(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__flush__catchAll(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__release(JNIEnv *env, jobject thiz);
void JJKC_AudioTrack__release__catchAll(JNIEnv *env, jobject thiz);
jint JJKC_AudioTrack__write(JNIEnv *env, jobject thiz, jbyteArray audioData, jint offsetInBytes, jint sizeInBytes);
jint JJKC_AudioTrack__write__catchAll(JNIEnv *env, jobject thiz, jbyteArray audioData, jint offsetInBytes, jint sizeInBytes);
jint JJKC_AudioTrack__setStereoVolume(JNIEnv *env, jobject thiz, jfloat leftGain, jfloat rightGain);
jint JJKC_AudioTrack__setStereoVolume__catchAll(JNIEnv *env, jobject thiz, jfloat leftGain, jfloat rightGain);
jint JJKC_AudioTrack__getAudioSessionId(JNIEnv *env, jobject thiz);
jint JJKC_AudioTrack__getAudioSessionId__catchAll(JNIEnv *env, jobject thiz);
int JJK_loadClass__JJKC_AudioTrack(JNIEnv *env);

#endif//JJK__android_media_AudioTrack__H
