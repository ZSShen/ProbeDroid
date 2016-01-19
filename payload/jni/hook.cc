
#include "globals.h"
#include "logcat.h"
#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "gadget.h"
#include "java/lang/stringbuilder.h"
#include "java/lang/string.h"


using namespace art;

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nHook success, pid = %d\n\n", getpid());

    JNIEnv *env;
    GetJniEnv(&env);

    // TODO: Currently, only simple gadgets for:
    //   StringBuilder.toString()
    //   String.indexOf(Ljava/lang/String;I)
    // are applied. In the long tern, we should design a gadget compiler to build
    // the gadgets for multiple hook points.

    // NOTE: Currently, the gadget naming is long and annoying. It aims to
    // fullfil the idiom of Java method signature. In the future, a more
    // systematic approach will be applied.

    Gadget_java_lang_StringBuilder gadget_stringbuilder;
    gadget_stringbuilder.toStringPatcher(env);
    //Gadget_java_lang_String gadget_string;
    //gadget_string.indexOf_javalangString_I_Patcher(env);
}
