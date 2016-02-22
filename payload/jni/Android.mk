LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE            := hook

LOCAL_C_INCLUDES        +=  ../../common \
                            ../../android/art/runtime \
                            ./gadget

LOCAL_SRC_FILES         :=  ../../common/utf.cc \
                            ./arch/x86/gadget_x86.s \
                            hook.cc \

LOCAL_CFLAGS            := -g -fexceptions

LOCAL_SHARED_LIBRARIES  := dl

LOCAL_LDLIBS            +=  -L$(SYSROOT)/usr/lib \
                            -llog \

LOCAL_CPP_EXTENSION     := .cxx .cpp .cc

include $(BUILD_SHARED_LIBRARY)
