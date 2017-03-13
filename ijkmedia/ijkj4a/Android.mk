# copyright (c) 2016 Zhang Rui <bbcallen@gmail.com>
#
# This file is part of ijkPlayer.
#
# ijkPlayer is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# ijkPlayer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with ijkPlayer; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH))

LOCAL_SRC_FILES += j4a/j4a_allclasses.c
LOCAL_SRC_FILES += j4a/j4a_base.c
LOCAL_SRC_FILES += j4a/class/android/media/AudioTrack.c
LOCAL_SRC_FILES += j4a/class/android/media/MediaCodec.c
LOCAL_SRC_FILES += j4a/class/android/media/MediaFormat.c
LOCAL_SRC_FILES += j4a/class/android/media/PlaybackParams.c
LOCAL_SRC_FILES += j4a/class/android/os/Build.c
LOCAL_SRC_FILES += j4a/class/android/os/Bundle.c
LOCAL_SRC_FILES += j4a/class/java/nio/Buffer.c
LOCAL_SRC_FILES += j4a/class/java/nio/ByteBuffer.c
LOCAL_SRC_FILES += j4a/class/java/util/ArrayList.c
LOCAL_SRC_FILES += j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c
LOCAL_SRC_FILES += j4a/class/tv/danmaku/ijk/media/player/misc/IIjkIOHttp.c
LOCAL_SRC_FILES += j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.c

LOCAL_SRC_FILES += j4au/class/android/media/AudioTrack.util.c
LOCAL_SRC_FILES += j4au/class/java/nio/ByteBuffer.util.c

LOCAL_MODULE := ijkj4a
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
