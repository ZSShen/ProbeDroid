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
