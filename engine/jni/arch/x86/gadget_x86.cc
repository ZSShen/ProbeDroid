/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

#include <memory>
#include <cstdlib>

#include "gadget_x86.h"
#include "gadget.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "ffi.h"
#include "globals.h"
#include "jni_except-inl.h"


// The buffer to cache the prologue of Thread::CreateInternalStackTrace.
uint8_t g_prologue_original_stack_trace[kCacheSizeDWord];

// The dummy code to replace the original prologue.
uint8_t g_prologue_hooked_stack_trace[kCacheSizeDWord] =
    {   0x31, 0xc0, // xor %eax, %eax
        0xc3,       // ret
        0x90        // nop
    };


void CloseRuntimeStackTrace()
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(g_create_internal_stack_trace);
    g_prologue_original_stack_trace[0] = ptr[0];
    g_prologue_original_stack_trace[1] = ptr[1];
    g_prologue_original_stack_trace[2] = ptr[2];
    g_prologue_original_stack_trace[3] = ptr[3];
    ptr[0] = g_prologue_hooked_stack_trace[0];
    ptr[1] = g_prologue_hooked_stack_trace[1];
    ptr[2] = g_prologue_hooked_stack_trace[2];
    ptr[3] = g_prologue_hooked_stack_trace[3];
}

void OpenRuntimeStackTrace()
{
    uint8_t* ptr = reinterpret_cast<uint8_t*>(g_create_internal_stack_trace);
    ptr[0] = g_prologue_original_stack_trace[0];
    ptr[1] = g_prologue_original_stack_trace[1];
    ptr[2] = g_prologue_original_stack_trace[2];
    ptr[3] = g_prologue_original_stack_trace[3];
}

void InputMarshaller::Extract(const std::vector<char>& input_type, void** arguments)
{
    off_t idx = 0;
    for (char type : input_type) {
        switch (type) {
            case kTypeBoolean:
            case kTypeByte:
            case kTypeChar:
            case kTypeShort:
            case kTypeInt:
            case kTypeObject:
                if (idx == 0)
                    *arguments++ = edx_;
                else if (idx == 1)
                    *arguments++ = ebx_;
                else
                    *arguments++ = *stack_++;
                ++idx;
                break;
            case kTypeFloat: {
                void* cast[1];
                jdouble value;
                if (idx == 0) {
                    *cast = edx_;
                    value = *reinterpret_cast<jfloat*>(cast);
                }
                else if(idx == 1) {
                    *cast = ebx_;
                    value = *reinterpret_cast<jfloat*>(cast);
                }
                else {
                    *cast = *stack_++;
                    value = *reinterpret_cast<jfloat*>(cast);
                }
                *reinterpret_cast<jdouble*>(arguments) = value;
                arguments += kWidthQword;
                ++idx;
                break;
            }
            case kTypeLong:
            case kTypeDouble:
                if (idx == 0) {
                    *arguments++ = edx_;
                    *arguments++ = ebx_;
                } else if (idx == 1) {
                    *arguments++ = ebx_;
                    *arguments++ = *stack_++;
                } else {
                    *arguments++ = *stack_++;
                    *arguments++ = *stack_++;
                }
                idx += kWidthQword;
                break;
        }
    }
}

void OutputMarshaller::Inject(char output_type, void** value)
{
    switch (output_type) {
        case kTypeVoid:
            *reinterpret_cast<uint32_t*>(ret_format_) = kNoData;
            *ret_value_ = *value;
            break;
        case kTypeBoolean:
        case kTypeByte:
        case kTypeChar:
        case kTypeShort:
        case kTypeInt:
        case kTypeObject:
            *reinterpret_cast<uint32_t*>(ret_format_) = kDwordInt;
            *ret_value_ = *value;
            break;
        case kTypeFloat:
            *reinterpret_cast<uint32_t*>(ret_format_) = kDwordFloat;
            *ret_value_ = *value;
            break;
        case kTypeLong:
            *reinterpret_cast<uint32_t*>(ret_format_) = kQwordLong;
            *ret_value_++ = *value++;
            *ret_value_ = *value;
            break;
        case kTypeDouble:
            *reinterpret_cast<uint32_t*>(ret_format_) = kQwordDouble;
            *ret_value_++ = *value++;
            *ret_value_ = *value;
            break;
    }
}