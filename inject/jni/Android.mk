LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := inject

LOCAL_C_INCLUDES        := ../../common

LOCAL_SRC_FILES         := ../../common/log.cc \
						   ../../common/stringprintf.cc \
						   inject.cc \
						   proc.cc

LOCAL_CFLAGS            := -g -fexceptions

LOCAL_SHARED_LIBRARIES  := dl

include $(BUILD_EXECUTABLE)
