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

set(SDL_DESKTOP_UNI_SOURCES
    desktop/ijksdl_desktop.h
    desktop/ijksdl_aout_port_audio.h    
    desktop/ijksdl_aout_port_audio.c
    desktop/ijksdl_aout_sdl2_audio.c
    desktop/ijksdl_aout_sdl2_audio.h
    desktop/ijksdl_vout_uni_callback.c
    desktop/ijksdl_vout_uni_callback.h
    desktop/ijksdl_desktop_vout_glfw.c
    desktop/ijksdl_desktop_vout_glfw.h
    desktop/ijksdl_desktop_gl.c
    desktop/ijksdl_desktop_gl.h
    desktop/ijksdl_vout_sdl2_video.c
    desktop/ijksdl_vout_sdl2_video.h
)

set(SDL_MAC_OSX_SOURCES
    ${IJKROOT_PATH}/ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_thread_ios.h
    ${IJKROOT_PATH}/ios/IJKMediaPlayer/IJKMediaPlayer/ijkmedia/ijksdl/ios/ijksdl_thread_ios.m
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
    list(APPEND GLES2_SOURCES gles2/renderer_amc.c)
    list(APPEND GLES2_SOURCES gles2/fsh/amc.fsh.c)
    list(APPEND GLES2_SOURCES gles2/vsh/amc.vsh.c)
    set(IJK_SDL_SOURCES ${IJK_SDL_SOURCES} ${SDL_ANDROID_EXTRA_SOURCES} ${GLES2_SOURCES})
endif ()

if (IJK_DESKTOP_UNI)
    find_package(SDL2 REQUIRED)

    set(IJK_SDL_SOURCES ${IJK_SDL_SOURCES} ${SDL_DESKTOP_UNI_SOURCES} ${GLES2_SOURCES})
    if (APPLE AND IJK_DESKTOP_UNI)
        set(IJK_SDL_SOURCES ${IJK_SDL_SOURCES} ${SDL_MAC_OSX_SOURCES})
        # list(APPEND IJK_SDL_SOURCES gles2/renderer_yuv420sp.c)
        # list(APPEND IJK_SDL_SOURCES gles2/fsh/yuv420sp.fsh.c)

    endif ()
    add_library(ijksdl_static STATIC ${IJK_SDL_SOURCES})
    set(IJKSDL_LIB_NAME ijksdl_static)

    target_include_directories(ijksdl_static PRIVATE ${SDL2_INCLUDE_DIRS})
    cmake_print_variables(SDL2_INCLUDE_DIRS)

    target_include_directories(ijksdl_static PRIVATE ${IJKROOT_PATH}/desktop/glfw/include)
    # target_include_directories(ijksdl_static PRIVATE ${IJKROOT_PATH}/desktop/glad/include)
    target_include_directories(ijksdl_static PRIVATE ${IJKROOT_PATH}/desktop/glfw/deps)
else ()
    add_library(ijksdl SHARED ${IJK_SDL_SOURCES})
    set(IJKSDL_LIB_NAME ijksdl)
endif ()

target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)

if(IJK_DESKTOP_UNI)
    target_include_directories(${IJKSDL_LIB_NAME} PRIVATE "${IJKROOT_PATH}/ijkmedia/portaudio/include")
    target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ijkyuv/include)
endif()

if (WIN32)
    target_include_directories(${IJKSDL_LIB_NAME} PRIVATE "${IJK_DESKTOP_PTHREAD_BUILD_DIR}/../../include")
endif()

#include ffmpeg header files
target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${IJK_FFMPAG_BUILD_DIR}/include)

if (ANDROID)
    # build cpufeatures as a static lib
    # add_library(cpufeatures STATIC
    #        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

    # include(AndroidNdkModules)
    # android_ndk_import_module_cpufeatures()
    target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ijkyuv/include)
    target_include_directories(${IJKSDL_LIB_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../ijkj4a)
endif ()

if (ANDROID)
    target_link_libraries(${IJKSDL_LIB_NAME} ijkffmpeg)
    target_link_libraries(${IJKSDL_LIB_NAME} ijkj4a)
    # target_link_libraries(ijksdl cpufeatures)
    target_link_libraries(${IJKSDL_LIB_NAME} yuv_static)
    target_link_libraries(${IJKSDL_LIB_NAME} log android OpenSLES EGL GLESv2)
endif ()

