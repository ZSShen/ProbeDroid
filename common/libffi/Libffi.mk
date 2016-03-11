# Copyright 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# The libffi code is organized primarily by architecture, but at some
# point OS-specific issues started to creep in. In some cases there
# are OS-specific source files, in others there are just #ifdefs in
# the code. We need to generate the appropriate defines and select the
# right set of source files for the OS and architecture.

LOCAL_C_INCLUDES := \
	external/libffi/include \
	external/libffi/$(ffi_os)-$(ffi_arch)

ifeq ($(ffi_os)-$(ffi_arch),linux-arm)
  LOCAL_SRC_FILES := src/arm/sysv.S src/arm/ffi.c
endif

ifeq ($(ffi_os)-$(ffi_arch),linux-mips)
  LOCAL_SRC_FILES += src/mips/ffi.c src/mips/o32.S
endif

ifeq ($(ffi_os)-$(ffi_arch),linux-x86)
  LOCAL_SRC_FILES := src/x86/ffi.c src/x86/sysv.S
endif

ifeq ($(ffi_os)-$(ffi_arch),linux-x86_64)
  LOCAL_SRC_FILES := src/x86/ffi64.c src/x86/unix64.S
endif

ifeq ($(ffi_os)-$(ffi_arch),darwin-x86)
  LOCAL_SRC_FILES := src/x86/ffi.c src/x86/darwin.S
endif

ifeq ($(LOCAL_SRC_FILES),)
  $(info The os/architecture $(ffi_os)-$(ffi_arch) is not supported by libffi.)
  LOCAL_SRC_FILES := your-architecture-not-supported-by-ffi-makefile.c
endif

LOCAL_SRC_FILES += \
	src/debug.c \
	src/java_raw_api.c \
	src/prep_cif.c \
        src/raw_api.c \
	src/types.c
