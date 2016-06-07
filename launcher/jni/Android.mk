LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := launcher

LOCAL_C_INCLUDES        := ../../common

LOCAL_SRC_FILES         := ../../common/log.cc \
                           ../../common/stringprintf.cc \
                           launcher.cc \
                           proc_common.cc \

ifeq ($(TARGET_ARCH),arm)
    LOCAL_SRC_FILES     += ./arch/proc_arm.cc
else
    LOCAL_SRC_FILES     += ./arch/proc_x86.cc
endif

LOCAL_CFLAGS            := -g -fexceptions

LOCAL_SHARED_LIBRARIES  := dl

include $(BUILD_EXECUTABLE)
