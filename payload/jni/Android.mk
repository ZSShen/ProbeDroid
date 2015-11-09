LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE                := hook
LOCAL_SRC_FILES             := hook.cc
LOCAL_CFLAGS                := -g
LOCAL_SHARED_LIBRARIES      := dl
LOCAL_LDLIBS                += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)
