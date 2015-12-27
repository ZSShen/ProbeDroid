#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cstring>

#include "getopt.h"
#include "unistd.h"
#include "fcntl.h"
#include "errno.h"
#include "dlfcn.h"
#include "sys/wait.h"
#include "sys/syscall.h"
#include "sys/ptrace.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/user.h"
#include "sys/mman.h"
#include "android/log.h"


namespace util {

#define SIZE_TINY_BLAH              (0x40)
#define SIZE_MID_BLAH               (0x200)

#define SUCC                        (0)
#define FAIL                        (1)

#define ERROR(...)      SpewMsg(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG(...)        SpewMsg(__VA_ARGS__)

void SpewMsg(const char*, const char*, const int32_t, const char*, ...);
void SpewMsg(const char*, ...);


class BadProbe : public std::exception
{
  public:
    const char* what() const throw ()
    {
        return "Fail to interact with the probed process.";
    }
};

class BadSharedObject : public std::exception
{
  public:
    const char* what() const throw()
    {
        return "Fail to load the designated shared object.";
    }
};

class DlHandle
{
  private:
    void* handle_;

  public:
    DlHandle(const char* sz_path, int32_t flag)
    {
        handle_ = dlopen(sz_path, flag);
        if (!handle_)
            throw BadSharedObject();
    }

    uint32_t ResolveSymbol(const char* symbol)
    {
        // Expect type coercion. But better approach is needed.
        return (uint32_t)dlsym(handle_, symbol);
    }

    ~DlHandle()
    {
        dlclose(handle_);
    }
};

}

#endif