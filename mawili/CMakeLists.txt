
#
# Copyright (c) 2019 befovy <befovy@gmail.com>
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
#

cmake_minimum_required(VERSION 3.6)

project("IjkPlayer")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")

set(IJKROOT_PATH "${PROJECT_SOURCE_DIR}/..")
get_filename_component(IJKROOT_PATH ${IJKROOT_PATH} ABSOLUTE)
message(STATUS "IJKROOT_PATH ${IJKROOT_PATH}")


if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_definitions(-DNDEBUG)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG)
endif ()


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    message(STATUS "CMAKE SIZEOF POIONT 8")
    set(IJK_X64 on)
    set(IJK_X86 off)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    message(STATUS "cmake sizeof point 4")
    set(IJK_X64 off)
    set(IJK_X86 on)
endif()

 

set(IJK_DESKTOP_CONTRIB_PATH ${IJKROOT_PATH}/mawili/contrib)

if(IJK_X64) 
    set(IJK_FFMPAG_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/ffmpeg/x86)
elseif(IJK_X86)
    set(IJK_FFMPAG_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/ffmpeg/x64)
endif()

if (WIN32)
    if(IJK_X64) 
        set(IJK_DESKTOP_PTHREAD_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/pthreads-w32-2-9-1-release/Pre-built.2/lib/x64)
    elseif(IJK_X86)
        set(IJK_DESKTOP_PTHREAD_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86)
    endif()

    add_definitions(-DHAVE_STRUCT_TIMESPEC)
endif()

if (NOT DEFINED IJK_FFMPAG_BUILD_DIR)
    message(FATAL_ERROR "Can not find ffmpeg build output: ${IJK_FFMPAG_BUILD_DIR}")
else ()
    message(STATUS "IJK_FFMPAG_BUILD_DIR:${IJK_FFMPAG_BUILD_DIR}")
endif ()

# find prebuild static ffmepg library
# add ffmpeg include path
# add ffmpeg link path
# link ffmpeg libs to target

add_definitions(-DIJK_DESKTOP_MWL)
set(IJK_DESKTOP_MWL ON)
# add_library(ijkplayerall SHARED )

add_subdirectory(../ijkmedia ijkmedia)

