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

#include "gadget_arm.h"
#include "gadget.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "ffi.h"
#include "globals.h"
#include "jni_except-inl.h"


void* ComposeInstrumentGadget(void *r1, void *r0, void *r2)
{
    return nullptr;
}

void ArtQuickInstrument(void** ret_format, void** ret_value, void* r1, void* r0,
                        void* r2, void* r3, void** stack)
{
}

void CloseRuntimeStackTrace()
{
}

void OpenRuntimeStackTrace()
{
}

void InputMarshaller::Extract(const std::vector<char>& input_type, void** arguments)
{
}

void OutputMarshaller::Inject(char output_type, void** value)
{
}
