#ifndef _LOGCAT_H_
#define _LOGCAT_H_


#include "log.h"
#include "macros.h"


#define CAT(severity) SpewCat(__FILE__, __LINE__, severity).stream()

class SpewCat
{
  public:
    SpewCat(const char* file, int line, LogSeverity severity)
     : severity_(severity),
       line_(line),
       file_(file)
    {}

    ~SpewCat();

    std::ostream& stream()
    {
        return buffer_;
    }

  private:
    LogSeverity severity_;
    int line_;
    const char* file_;
    std::ostringstream buffer_;

    DISALLOW_COPY_AND_ASSIGN(SpewCat);
};


#endif