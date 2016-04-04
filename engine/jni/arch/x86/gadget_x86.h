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

#ifndef _GADGET_X86_H_
#define _GADGET_X86_H_


#include <cstdint>
#include <vector>
#include <memory>
#include <jni.h>

#include "indirect_reference_table.h"
#include "jni_internal.h"
#include "ffi.h"


class InputMarshaller
{
  public:
    InputMarshaller(void* ecx, void* eax, void* ebx, void* edx, void** stack)
      : ecx_(ecx),
        eax_(eax),
        ebx_(ebx),
        edx_(edx),
        stack_(stack + kStackAlignment)
    {}

    void Extract(const std::vector<char>& input_type, void**);

    void* GetReceiver()
    {
        return ecx_;
    }

    jmethodID GetMethodID()
    {
        return reinterpret_cast<jmethodID>(eax_);
    }

  private:
    static const constexpr int32_t kStackAlignment = 5 + 3 + 1;

    void* ecx_;
    void* eax_;
    void* edx_;
    void* ebx_;
    void** stack_;
};

class OutputMarshaller
{
  public:
    OutputMarshaller(void** ret_format, void** ret_value)
      : ret_format_(ret_format),
        ret_value_(ret_value)
    {}

    void Inject(char, void**);

  private:
    void** ret_format_;
    void** ret_value_;
};

#endif