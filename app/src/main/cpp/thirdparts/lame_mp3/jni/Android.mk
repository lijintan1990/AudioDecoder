LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_LST := $(wildcard $(LOCAL_PATH)/../lame-3.100/libmp3lame/*.c)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../lame-3.100/include $(LOCAL_PATH)/../lame-3.100 $(LOCAL_PATH)/../lame-3.100/libmp3lame

LOCAL_MODULE    := mp3lame
LOCAL_SRC_FILES := $(SRC_LST)

include $(BUILD_SHARED_LIBRARY)
