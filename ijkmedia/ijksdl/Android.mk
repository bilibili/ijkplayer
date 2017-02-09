# Copyright (c) 2013 Bilibili
# copyright (c) 2013 Zhang Rui <bbcallen@gmail.com>
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
LOCAL_LDLIBS += -llog -landroid -lOpenSLES -lEGL -lGLESv2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/../ijkyuv/include)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/../ijkj4a)

LOCAL_SRC_FILES += ijksdl_aout.c
LOCAL_SRC_FILES += ijksdl_audio.c
LOCAL_SRC_FILES += ijksdl_egl.c
LOCAL_SRC_FILES += ijksdl_error.c
LOCAL_SRC_FILES += ijksdl_mutex.c
LOCAL_SRC_FILES += ijksdl_stdinc.c
LOCAL_SRC_FILES += ijksdl_thread.c
LOCAL_SRC_FILES += ijksdl_timer.c
LOCAL_SRC_FILES += ijksdl_vout.c
LOCAL_SRC_FILES += gles2/color.c
LOCAL_SRC_FILES += gles2/common.c
LOCAL_SRC_FILES += gles2/renderer.c
LOCAL_SRC_FILES += gles2/renderer_rgb.c
LOCAL_SRC_FILES += gles2/renderer_yuv420p.c
LOCAL_SRC_FILES += gles2/renderer_yuv444p10le.c
LOCAL_SRC_FILES += gles2/shader.c
LOCAL_SRC_FILES += gles2/fsh/rgb.fsh.c
LOCAL_SRC_FILES += gles2/fsh/yuv420p.fsh.c
LOCAL_SRC_FILES += gles2/fsh/yuv444p10le.fsh.c
LOCAL_SRC_FILES += gles2/vsh/mvp.vsh.c

LOCAL_SRC_FILES += dummy/ijksdl_vout_dummy.c

LOCAL_SRC_FILES += ffmpeg/ijksdl_vout_overlay_ffmpeg.c
LOCAL_SRC_FILES += ffmpeg/abi_all/image_convert.c

LOCAL_SRC_FILES += android/android_audiotrack.c
LOCAL_SRC_FILES += android/android_nativewindow.c
LOCAL_SRC_FILES += android/ijksdl_android_jni.c
LOCAL_SRC_FILES += android/ijksdl_aout_android_audiotrack.c
LOCAL_SRC_FILES += android/ijksdl_aout_android_opensles.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediacodec_dummy.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediacodec_internal.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediacodec_java.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediacodec.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediadef.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediaformat_java.c
LOCAL_SRC_FILES += android/ijksdl_codec_android_mediaformat.c
LOCAL_SRC_FILES += android/ijksdl_vout_android_nativewindow.c
LOCAL_SRC_FILES += android/ijksdl_vout_android_surface.c
LOCAL_SRC_FILES += android/ijksdl_vout_overlay_android_mediacodec.c

LOCAL_SHARED_LIBRARIES := ijkffmpeg
LOCAL_STATIC_LIBRARIES := cpufeatures yuv_static ijkj4a

LOCAL_MODULE := ijksdl
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
