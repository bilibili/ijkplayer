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
LOCAL_C_CFLAGS := -std=c99
LOCAL_SRC_FILES := loghelper.c
LOCAL_MODULE := helpers_c
include $(BUILD_STATIC_LIBRARY)


#--------------------
# CPP files
# to suppress ugly c99 warning
#--------------------
include $(CLEAR_VARS)
LOCAL_SRC_FILES := JNIHelp.cpp
LOCAL_MODULE := helpers_cpp
include $(BUILD_STATIC_LIBRARY)


#--------------------
# so
#--------------------
include $(CLEAR_VARS)
LOCAL_SRC_FILES := JNIHelp.cpp

LOCAL_LDLIBS += -llog
LOCAL_MODULE := helpers

LOCAL_STATIC_LIBRARIES := helpers_c helpers_cpp
LOCAL_SHARED_LIBRARIES := libstlport

include $(BUILD_SHARED_LIBRARY)