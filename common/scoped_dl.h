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

#ifndef _UTIL_SCOPED_DL_H_
#define _UTIL_SCOPED_DL_H_


#include <dlfcn.h>

#include "globals.h"
#include "macros.h"


class ScopedDl
{
  public:
    explicit ScopedDl(void* dl)
      : dl_(dl)
    {}

    ~ScopedDl()
    {
        reset();
    }

    void* get()
    {
        return dl_;
    }

    void reset(void* new_dl = nullptr)
    {
        if (dl_)
            dlclose(dl_);
        dl_ = new_dl;
    }

    void* resolve(const char* symbol)
    {
        if (!dl_)
            return nullptr;
        return dlsym(dl_, symbol);
    }

  private:
    void* dl_;

    DISALLOW_COPY_AND_ASSIGN(ScopedDl);
};

#endif
