#ifndef _PROC_H_
#define _PROC_H_


#include <sys/types.h>

#include "globals.h"


namespace proc {

class EggHunter;

class FunctionTable
{
  public:
    FunctionTable()
      : dlopen_(0), mmap_(0)
    {}

    ~FunctionTable()
    {}

    bool Resolve(pid_t, pid_t);

    uintptr_t GetDlopen() const
    {
        return dlopen_;
    }

    uintptr_t GetMmap() const
    {
        return mmap_;
    }

  private:
    uintptr_t GetLibraryAddress(pid_t, const char*);
    uintptr_t GetFunctionAddress(const char*, const char*);

    uintptr_t dlopen_;
    uintptr_t mmap_;
};

class EggHunter
{
  public:
    EggHunter()
      : pid_app_(0), func_tbl_()
    {}

    ~EggHunter()
    {}

    void Hunt(pid_t, const char*, const char*, const char*, const char*);

  private:
    bool CaptureApp(pid_t, const char*);
    bool InjectApp(const char*, const char*, const char*);
    void WaitForForkEvent(pid_t);
    void CheckStartupCmd(const char*);
    bool PokeTextInApp(uintptr_t, const char*, size_t);
    bool PeekTextInApp(uintptr_t, char*, size_t);

    pid_t pid_app_;
    FunctionTable func_tbl_;
};

}

#endif