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

#--------------------
# C files
#--------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)

# LOCAL_SRC_FILES += ijkadk.c

LOCAL_SHARED_LIBRARIES := ijkutil

LOCAL_MODULE := ijkadk_c
include $(BUILD_STATIC_LIBRARY)


#--------------------
# CPP files: android
#--------------------
include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)

LOCAL_SRC_FILES += ijkadk_android_media_MediaCodec.cpp
LOCAL_SRC_FILES += ijkadk_java_lang_String.cpp
LOCAL_SRC_FILES += ijkadk_java_nio_ByteBuffer.cpp
LOCAL_SRC_FILES += ijkadkinternal.cpp
LOCAL_SRC_FILES += ijkadkobject.cpp

LOCAL_SRC_FILES += ijkadkfoundation.cpp

LOCAL_SHARED_LIBRARIES := ijkutil

LOCAL_MODULE := ijkadk_cpp
include $(BUILD_STATIC_LIBRARY)


#--------------------
# so
#--------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS += -std=c99
LOCAL_LDLIBS += -llog -landroid

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(realpath $(LOCAL_PATH)/..)

LOCAL_SRC_FILES += ijkadk.c

LOCAL_SHARED_LIBRARIES := ijkutil
LOCAL_WHOLE_STATIC_LIBRARIES := ijkadk_c ijkadk_cpp

LOCAL_MODULE := ijkadk
include $(BUILD_SHARED_LIBRARY)
