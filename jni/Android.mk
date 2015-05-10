LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := jpeg-ndk
LOCAL_SRC_FILES := jpeg-ndk.c rrimagelib.c
LOCAL_LDLIBS    := -llog -lm -ljnigraphics
LOCAL_STATIC_LIBRARIES += libjpeg
LOCAL_C_INCLUDES += $(LOCAL_PATH)/libjpeg
include $(BUILD_SHARED_LIBRARY)
include $(LOCAL_PATH)/libjpeg/Android.mk



