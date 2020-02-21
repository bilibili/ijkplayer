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

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    message(STATUS "CMAKE SIZEOF POIONT 8")
    set(IJK_X64 on)
    set(IJK_X86 off)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    message(STATUS "cmake sizeof point 4")
    set(IJK_X64 off)
    set(IJK_X86 on)
endif ()


set(IJK_DESKTOP_CONTRIB_PATH ${IJKROOT_PATH}/desktop/contrib)

if (IJK_X64)
    if (APPLE) 
        set(IJK_FFMPAG_BUILD_DIR ${IJKROOT_PATH}/osx/contrib/build/)
    else()
        set(IJK_FFMPAG_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/ffmpeg/x64)
    endif()
elseif (IJK_X86)
    set(IJK_FFMPAG_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/ffmpeg/x86)
endif ()

if (WIN32)
    if (IJK_X64)
        set(IJK_DESKTOP_PTHREAD_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/pthreads-w32-2-9-1-release/Pre-built.2/lib/x64)
    elseif (IJK_X86)
        set(IJK_DESKTOP_PTHREAD_BUILD_DIR ${IJK_DESKTOP_CONTRIB_PATH}/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86)
    endif ()

    add_definitions(-DHAVE_STRUCT_TIMESPEC)
endif ()

if (NOT DEFINED IJK_FFMPAG_BUILD_DIR)
    message(FATAL_ERROR "Can not find ffmpeg build output: ${IJK_FFMPAG_BUILD_DIR}")
else ()
    message(STATUS "IJK_FFMPAG_BUILD_DIR:${IJK_FFMPAG_BUILD_DIR}")
endif ()


add_definitions(-DIJK_DESKTOP_UNI)
set(IJK_DESKTOP_UNI ON)

set(FIJKPLAYER_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/sources/ijkplayer_desktop.h
    ${CMAKE_CURRENT_LIST_DIR}/sources/ijkplayer_desktop.c
)

add_library(IjkPlayer SHARED ${FIJKPLAYER_SOURCE})

add_subdirectory(../ijkmedia ijkmedia)

target_include_directories(IjkPlayer PRIVATE ${IJKROOT_PATH}/ijkmedia)
target_include_directories(IjkPlayer PRIVATE ${IJK_FFMPAG_BUILD_DIR}/include)
target_include_directories(IjkPlayer PRIVATE "${IJK_DESKTOP_PTHREAD_BUILD_DIR}/../../include")

target_link_libraries(IjkPlayer ijksdl_static ijkplayer_static yuv)


if (WIN32)
    target_compile_options(ijksdl_static PRIVATE /W3 /we4013)
    target_compile_options(ijkplayer_static PRIVATE /W3 /we4013)
    target_compile_options(IjkPlayer PRIVATE /W3 /we4013)
endif ()

find_package(PkgConfig REQUIRED)

if (APPLE)
    # set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${IJKROOT_PATH}/osx/contrib/build")
    # set(ENV{PKG_CONFIG_PATH} "${IJKROOT_PATH}/osx/contrib/build")
    set(CMAKE_PREFIX_PATH ${IJKROOT_PATH}/osx/contrib/build)
    pkg_check_modules(OPENSSL_STATIC openssl)
    pkg_check_modules(SRT_STATIC REQUIRED srt)
    pkg_check_modules(FFMPEG_STATIC REQUIRED libavcodec libavformat libavutil libswresample libswscale)
endif ()

if (WIN32)
    add_library(pthread STATIC IMPORTED)
    set_target_properties(pthread PROPERTIES IMPORTED_LOCATION ${IJK_DESKTOP_PTHREAD_BUILD_DIR}/pthreadVC2.lib)
    target_link_libraries(IjkPlayer pthread)
endif ()

message(STATUS "ffmpeg libs:${FFMPEG_STATIC_LIBRARIES}")

target_link_libraries(IjkPlayer ${FFMPEG_STATIC_LIBRARIES})
target_link_directories(IjkPlayer PUBLIC ${FFMPEG_STATIC_LIBRARY_DIRS})
target_link_libraries(IjkPlayer portaudio_static)
if (SRT_FOUND)
    message(STATUS "SRT ${SRT_STATIC_LIBRARIES}")
    target_link_libraries(IjkPlayer ${SRT_STATIC_LIBRARIES})
    target_link_directories(IjkPlayer PUBLIC ${SRT_STATIC_LIBRARY_DIRS})
endif()

if(APPLE AND SRT_STATIC_FOUND)
    set_target_properties(IjkPlayer PROPERTIES LINK_FLAGS "-Wl,-no_compact_unwind")
endif()

target_compile_definitions(IjkPlayer PRIVATE -DBUILDING_IJK=1)

if (WIN32)
    target_link_libraries(IjkPlayer ws2_32 secur32 avrt shlwapi advapi32 shell32 kernel32 user32)
elseif (APPLE)
    # brew install openssl
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(IjkPlayer ${OPENSSL_LIBRARIES})
    target_link_libraries(IjkPlayer z)
    target_link_libraries(IjkPlayer bz2)

    macro(add_framework appname fwname)
        find_library(FRAMEWORK_${fwname}
                NAMES ${fwname}
                PATHS ${CMAKE_OSX_SYSROOT}/System/Library
                PATH_SUFFIXES Frameworks NO_DEFAULT_PATH)

        if (${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
            MESSAGE(ERROR ": Framework ${fwname} not found")
        else ()
            target_link_libraries(${appname} "${FRAMEWORK_${fwname}}")
            MESSAGE(STATUS "Framework ${fwname} found")
        endif ()
    endmacro(add_framework)

    add_framework(IjkPlayer CoreVideo)
    add_framework(IjkPlayer CoreMedia)
    add_framework(IjkPlayer VideoToolbox)
    add_framework(IjkPlayer Security)
endif ()

add_executable(tuidemo ${CMAKE_CURRENT_LIST_DIR}/sources/ijkplayer_demo.c)

if (WIN32)
    target_link_libraries(tuidemo IjkPlayer user32)
else ()
    target_link_libraries(tuidemo IjkPlayer)
endif ()

