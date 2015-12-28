#ifndef _GADGET_JAVA_LANG_STRING_H_
#define _GADGET_JAVA_LANG_STRING_H_


#include "gadget.h"


extern OriginalCall indexOf_javalangString_I_;
extern "C" void indexOf_javalangString_I_PreCall(void*, int);
extern "C" void indexOf_javalangString_I_PostCall(int);
extern "C" void indexOf_javalangString_I_Trampoline() \
        __asm__("indexOf_javalangString_I_Trampoline");

class Gadget_java_lang_String : public Gadget
{
  public:
    bool indexOf_javalangString_I_Patcher(JNIEnv*);
};

#endif