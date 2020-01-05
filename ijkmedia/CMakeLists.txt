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

cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED FIJKROOT_PATH)
    set(FIJKROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/../)
endif ()

if(IJK_DESKTOP_UNI)
    add_subdirectory(portaudio)
    add_subdirectory(ijkyuv)

    target_compile_options(portaudio_static PRIVATE "-Wno-deprecated-declarations")
    target_compile_options(portaudio PRIVATE "-Wno-deprecated-declarations")
endif()

add_subdirectory(ijksdl)
add_subdirectory(ijkplayer)

if (ANDROID)
    add_subdirectory(ijksoundtouch)
    add_subdirectory(ijkyuv)
    add_subdirectory(ijkj4a)
    # just a dummy android ndk prifiler
    option(ANDROID_NDK_PROFILER OFF)
    if (NDK_PROFILER)
        # not implemented yet
        message(FATAL_ERROR "not implemented yet, android-ndk-profiler")
        add_subdirectory(../ijkprof/android-ndk-profiler/jni ijkprof)
    else ()
        add_subdirectory(../ijkprof/android-ndk-profiler-dummy/jni ijkprof)
    endif (NDK_PROFILER)

    # add_dependencies(ijkplayer android-ndk-profiler)
endif ()
