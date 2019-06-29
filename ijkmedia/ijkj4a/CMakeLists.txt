#
# Copyright (c) 2019 Befovy <befovy@gmail.com>
#
# This file is part of fijkPlayer.
#
# fijkPlayer is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# fijkPlayer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with ijkPlayer; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#


cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(J4A_SOURCES
    j4a/j4a_allclasses.c
    j4a/j4a_base.c
    j4a/class/android/media/AudioTrack.c
    j4a/class/android/media/MediaCodec.c
    j4a/class/android/media/MediaFormat.c
    j4a/class/android/media/PlaybackParams.c
    j4a/class/android/os/Build.c
    j4a/class/android/os/Bundle.c
    j4a/class/java/nio/Buffer.c
    j4a/class/java/nio/ByteBuffer.c
    j4a/class/java/util/ArrayList.c
    j4a/class/tv/danmaku/ijk/media/player/misc/IMediaDataSource.c
    j4a/class/tv/danmaku/ijk/media/player/misc/IAndroidIO.c
    j4a/class/tv/danmaku/ijk/media/player/IjkMediaPlayer.c
    j4au/class/android/media/AudioTrack.util.c
    j4au/class/java/nio/ByteBuffer.util.c
)

add_library(ijkj4a STATIC ${J4A_SOURCES})
