
#include "util.h"
#include "logcat.h"
#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "gadget.h"
#include "java/lang/stringbuilder.h"


using namespace util;
using namespace art;

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nHook success, pid = %d\n\n", getpid());

    JNIEnv *env;
    GetJniEnv(&env);

    // TODO: Currently, only a simple gadget for StringBuilder.toString() is
    // is applied. In the long tern, we should design a gadget compiler to build
    // the gadgets for multiple hook points.
    Gadget_java_lang_StringBuilder gadget;
    bool result = gadget.toStringPatcher(env);
}
