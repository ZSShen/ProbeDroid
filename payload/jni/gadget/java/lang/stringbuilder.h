#ifndef _GADGET_JAVA_LANG_STRINGBUILDER_H_
#define _GADGET_JAVA_LANG_STRINGBUILDER_H_


#include "gadget.h"


extern OriginalCall toString;
extern "C" void toStringPreCall();
extern "C" void toStringPostCall(void*);
extern "C" void toStringTrampoline() __asm__("toStringTrampoline");

class Gadget_java_lang_StringBuilder : public Gadget
{
  public:
    bool toStringPatcher(JNIEnv*);
};

#endif