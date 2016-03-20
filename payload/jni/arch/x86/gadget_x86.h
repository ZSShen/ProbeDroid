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

    void Extract(int32_t, void**);

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
    OutputMarshaller()
    {}
};

#endif