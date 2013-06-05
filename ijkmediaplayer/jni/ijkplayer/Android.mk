LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH) $(MY_APP_FFMPEG_INCLUDE_PATH)
# LOCAL_LDLIBS += -ldl -llog

LOCAL_SRC_FILES := ijkplayer_jni.c
LOCAL_SRC_FILES += ijkplayer.c
LOCAL_SRC_FILES += pkt_queue.c
LOCAL_SRC_FILES += minisdl/minisdl_thread.c

LOCAL_SHARED_LIBRARIES := ffmpeg

LOCAL_MODULE := ijkplayer
include $(BUILD_SHARED_LIBRARY)