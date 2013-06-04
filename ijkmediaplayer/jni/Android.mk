LOCAL_PATH := $(call my-dir)

MY_APP_JNI_ROOT := $(realpath $(LOCAL_PATH))
MY_APP_PRJ_ROOT := $(realpath $(MY_APP_JNI_ROOT)/..)
MY_APP_REPO_ROOT := $(realpath $(MY_APP_PRJ_ROOT)/..)

# armeabi-v7a
MY_APP_FFMPEG_OUTPUT_PATH := $(realpath $(MY_APP_REPO_ROOT)/build/ffmpeg-armv7a/output)
MY_APP_FFMPEG_INCLUDE_PATH := $(realpath $(MY_APP_FFMPEG_OUTPUT_PATH)/include)

include $(call all-subdir-makefiles)
