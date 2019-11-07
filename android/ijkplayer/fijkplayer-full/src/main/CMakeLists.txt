#
# Copyright (c) 2019 befovy <befovy@gmail.com>
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

project("FijkPlayer")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(ANDROID_PLATFORM_LEVEL 16)


add_definitions(-DANDROID)


message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "ANDROID_PLATFORM_LEVEL:${ANDROID_PLATFORM_LEVEL}")
message(STATUS "ANDROID_ABI:${ANDROID_ABI}")
message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")

set(FIJKROOT_PATH "${PROJECT_SOURCE_DIR}/../../../../..")
get_filename_component(FIJKROOT_PATH ${FIJKROOT_PATH} ABSOLUTE)
message(STATUS "FIJKROOT_PATH ${FIJKROOT_PATH}")


if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif ()


set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(ANDROID_CONTRIB_PATH ${FIJKROOT_PATH}/android/contrib/build)
message(STATUS "ANDROID_CONTRIB_PATH:${ANDROID_CONTRIB_PATH}")

if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    set(IJK_FFMPAG_BUILD_DIR ${ANDROID_CONTRIB_PATH}/ffmpeg-arm64/output)
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(IJK_FFMPAG_BUILD_DIR ${ANDROID_CONTRIB_PATH}/ffmpeg-armv7a/output)
elseif (${ANDROID_ABI} STREQUAL "armeabi")
    set(IJK_FFMPAG_BUILD_DIR ${ANDROID_CONTRIB_PATH}/ffmpeg-armv5/output)
elseif (${ANDROID_ABI} STREQUAL "x86")
    set(IJK_FFMPAG_BUILD_DIR ${ANDROID_CONTRIB_PATH}/ffmpeg-x86/output)
elseif (${ANDROID_ABI} STREQUAL "x86_64")
    set(IJK_FFMPAG_BUILD_DIR ${ANDROID_CONTRIB_PATH}/ffmpeg-x86_64/output)
endif ()

if (NOT DEFINED IJK_FFMPAG_BUILD_DIR)
    message(FATAL_ERROR "Can not find ffmpeg build output: ${IJK_FFMPAG_BUILD_DIR}")
else ()
    message(STATUS "IJK_FFMPAG_BUILD_DIR:${IJK_FFMPAG_BUILD_DIR}")
endif ()

add_library(ijkffmpeg SHARED IMPORTED)
set_target_properties( # Specifies the target library.
    ijkffmpeg
    PROPERTIES
    IMPORTED_LOCATION ${IJK_FFMPAG_BUILD_DIR}/libijkffmpeg.so
)

add_custom_target(cpffmpeg
    COMMAND mkdir -p ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    COMMAND cp ${IJK_FFMPAG_BUILD_DIR}/libijkffmpeg.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_dependencies(ijkffmpeg cpffmpeg)

message(STATUE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${PROJECT_BINARY_DIR}")
add_subdirectory(${FIJKROOT_PATH}/ijkmedia ijkmedia)
