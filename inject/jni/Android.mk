LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := inject
LOCAL_C_INCLUDES        := ../../common
LOCAL_SRC_FILES         := ../../common/util.cc inject.cc
LOCAL_CFLAGS            := -g
LOCAL_SHARED_LIBRARIES  := dl

include $(BUILD_EXECUTABLE)
