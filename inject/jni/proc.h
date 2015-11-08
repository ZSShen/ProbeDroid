#ifndef _PROC_H_
#define _PROC_H_

#include <util.h>


namespace proc {

class EggHunter;

class FunctionTable
{
  private:
    uint32_t dlopen_;
    uint32_t mmap_;

    uint32_t GetLibBgnAddr(pid_t, const char*);
    uint32_t GetFuncBgnAddr(const char*, const char*);

  public:
    FunctionTable()
      : dlopen_(0), mmap_(0)
    {}

    ~FunctionTable()
    {}

    int32_t Resolve(pid_t, pid_t);

    uint32_t GetDlopen() const
    {
        return dlopen_;
    }

    uint32_t GetMmap() const
    {
        return mmap_;
    }
};

class EggHunter
{
  private:
    pid_t pid_app_;
    FunctionTable func_tbl_;

    int32_t CaptureApp(pid_t, const char*);
    int32_t InjectApp(const char*);
    void WaitForForkEvent(pid_t);
    void CheckStartupCmd(const char*);

  public:
    EggHunter()
      : pid_app_(0), func_tbl_()
    {}

    ~EggHunter()
    {}

    int32_t Hunt(pid_t, const char*, const char*);
};

}

#endif