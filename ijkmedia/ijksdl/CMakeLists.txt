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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set(BASE_SOURCES
    ijksdl_class.h
    ijksdl_container.h
    ijksdl_log.h

    ijksdl_misc.h
    ijksdl_stdinc.h
    ijksdl_inc_internal.h
    ijksdl_aout.c
    ijksdl_aout.h
    ijksdl_aout_internal.h
    ijksdl_audio.c
    ijksdl_audio.h
    ijksdl_error.c
    ijksdl_error.h
    ijksdl_mutex.c
    ijksdl_mutex.h
    ijksdl_stdinc.c
    ijksdl_stdinc.h
    ijksdl_thread.c
    ijksdl_thread.h
    ijksdl_timer.c
    ijksdl_timer.h
    ijksdl_vout.c
    ijksdl_vout.h
    ijksdl_vout_internal.h

    ffmpeg/ijksdl_vout_overlay_ffmpeg.c
    ffmpeg/ijksdl_vout_overlay_ffmpeg.h
    ffmpeg/ijksdl_image_convert.h
    ffmpeg/ijksdl_inc_ffmpeg.h
    ffmpeg/abi_all/image_convert.c

    dummy/ijksdl_vout_dummy.c
    dummy/ijksdl_vout_dummy.h
    dummy/ijksdl_dummy.h
)

set(SDL_ANDROID_EXTRA_SOURCES
    ijksdl_extra_log.c
    ijksdl_extra_log.h
    android/android_audiotrack.c
    android/android_nativewindow.c
    android/ijksdl_android_jni.c
    android/ijksdl_aout_android_audiotrack.c
    android/ijksdl_aout_android_opensles.c
    android/ijksdl_codec_android_mediacodec_dummy.c
    android/ijksdl_codec_android_mediacodec_internal.c
    android/ijksdl_codec_android_mediacodec_java.c
    android/ijksdl_codec_android_mediacodec.c
    android/ijksdl_codec_android_mediadef.c
    android/ijksdl_codec_android_mediaformat_java.c
    android/ijksdl_codec_android_mediaformat.c
    android/ijksdl_vout_android_nativewindow.c
    android/ijksdl_vout_android_surface.c
    android/ijksdl_vout_overlay_android_mediacodec.c
)

set(GLES2_SOURCES
    ijksdl_egl.c
    gles2/color.c
    gles2/common.c
    gles2/renderer.c
    gles2/renderer_rgb.c
    gles2/renderer_yuv420p.c
    gles2/renderer_yuv444p10le.c
    gles2/shader.c
    gles2/fsh/rgb.fsh.c
    gles2/fsh/yuv420p.fsh.c
    gles2/fsh/yuv444p10le.fsh.c
    gles2/vsh/mvp.vsh.c
)

set(IJK_SDL_SOURCES ${BASE_SOURCES})

if (ANDROID)
    set(IJK_SDL_SOURCES ${IJK_SDL_SOURCES} ${SDL_ANDROID_EXTRA_SOURCES} ${GLES2_SOURCES})
endif ()

if (IJK_DESKTOP_MWL)
    add_library(ijksdl STATIC ${IJK_SDL_SOURCES})
else ()
    add_library(ijksdl SHARED ${IJK_SDL_SOURCES})
endif ()

target_include_directories(ijksdl PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(ijksdl PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)

if (WIN32)
    target_include_directories(ijksdl PRIVATE "${IJK_DESKTOP_PTHREAD_BUILD_DIR}/../../include")
endif()

#include ffmpeg header files
target_include_directories(ijksdl PRIVATE ${IJK_FFMPAG_BUILD_DIR}/include)

if (ANDROID)
    # build cpufeatures as a static lib
    # add_library(cpufeatures STATIC
    #        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

    # include(AndroidNdkModules)
    # android_ndk_import_module_cpufeatures()
    target_include_directories(ijksdl PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ijkyuv/include)
    target_include_directories(ijksdl PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ijkj4a)
endif ()

if (ANDROID)
    target_link_libraries(ijksdl ijkffmpeg)
    target_link_libraries(ijksdl ijkj4a)
    # target_link_libraries(ijksdl cpufeatures)
    target_link_libraries(ijksdl yuv_static)
    target_link_libraries(ijksdl log android OpenSLES EGL GLESv2)
endif ()

