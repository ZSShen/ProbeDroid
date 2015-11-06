#ifndef _PROC_H_
#define _PROC_H_

#include <util.h>


namespace proc {

class EggHunter;

class FunctionTable
{
  private:
    void* dlopen_;
    void* mmap_;

    void* GetLibBgnAddr(pid_t);
    void* GetFuncBgnAddr(const char*, const char*);

  public:
    FunctionTable()
      : dlopen_(NULL), mmap_(NULL)
    {}

    ~FunctionTable()
    {}

    void Resolve(pid_t, pid_t);
    void* GetDlopen() const;
    void* GetMmap() const;
};

class EggHunter
{
  private:
    pid_t pid_app_;
    FunctionTable func_tbl_;

    int32_t CaptureApp(pid_t, char*);
    int32_t InjectApp();
    void WaitForForkEvent(pid_t);
    void CheckStartupCmd(char*);

  public:
    EggHunter()
      : pid_app_(0), func_tbl_()
    {}

    ~EggHunter()
    {}

    int32_t Hunt(pid_t, char*);
};

}

#endif