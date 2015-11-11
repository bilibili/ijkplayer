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

#include "jjk_internal.h"
#include "ijksdl/android/jjk/c/java/nio/ByteBuffer.h"
#include "ijksdl/android/jjk/c/java/util/ArrayList.h"
#include "ijksdl/android/jjk/c/android/os/Build.h"
#include "ijksdl/android/jjk/c/android/media/AudioTrack.h"
#include "ijksdl/android/jjk/c/android/media/MediaCodec.h"
#include "ijksdl/android/jjk/c/android/media/MediaFormat.h"
#include "ijksdl/android/jjk/c/android/os/Bundle.h"
#include "ijksdl/android/jjk/c/tv/danmaku/ijk/media/player/IjkMediaPlayer.h"
#include "ijksdl/android/jjk/c/tv/danmaku/ijk/media/player/misc/IMediaDataSource.h"

#define JJK_LOAD_CLASS(class__) \
    do { \
        ret = JJK_loadClass__JJKC_##class__(env); \
        if (ret) \
            goto fail; \
    } while (0)

int JJK_LoadAll__catchAll(JNIEnv *env)
{
    int ret = 0;

    JJK_LOAD_CLASS(ByteBuffer); // java_nio_ByteBuffer
    JJK_LOAD_CLASS(ArrayList);  // java_util_ArrayList

    // must load before all other android classes
    JJK_LOAD_CLASS(android_os_Build);

    JJK_LOAD_CLASS(AudioTrack);  // android_media_AudioTrack
    JJK_LOAD_CLASS(MediaCodec);  // android_media_MediaCodec
    JJK_LOAD_CLASS(MediaFormat); // android_media_MediaFormat
    JJK_LOAD_CLASS(Bundle);      // android_os_Bundle

    JJK_LOAD_CLASS(IjkMediaPlayer);     // tv_danmaku_ijk_media_player_IjkMediaPlayer
    JJK_LOAD_CLASS(IMediaDataSource);   // tv_danmaku_ijk_media_player_misc_IMediaDataSource

fail:
    return ret;
}
