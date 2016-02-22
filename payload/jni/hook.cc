#include "globals.h"
#include "logcat.h"
#include "hook.h"
#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "gadget.h"
#include "java/lang/stringbuilder.h"
#include "java/lang/string.h"


namespace hook {

bool Penetrator::ResolveAnalysisModule()
{
    char buf[kBlahSizeMid];
    sprintf(buf, "/proc/self/maps");

    std::ifstream map(buf, std::ifstream::in);
    while (map.good() && !map.eof()) {
        map.getline(buf, kBlahSizeMid);

        if (!strstr(buf, kPermReadWrteExec))
            continue;
        uintptr_t addr_bgn;
        #if __WORDSIZE == 64
        sscanf(buf, "%lx-", &addr_bgn);
        #else
        sscanf(buf, "%x-", &addr_bgn);
        #endif

        // Try to read the module pathname.
        bool found = true;
        char* path = reinterpret_cast<char*>(addr_bgn);
        while (*path != 0)
            ++path;
        ++path;
        size_t len = strlen(kKeyPathAnalysisModule);
        for (size_t i = 0 ; i < len ; ++i) {
            if (kKeyPathAnalysisModule[i] != path[i]) {
                found = false;
                break;
            }
        }
        if (found) {
            module_path_ = path + len + 1;
            return HOOK_SUCCESS;
        }
    }
    return HOOK_FAILURE;
}

}

using namespace art;

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nHook success, pid = %d\n\n", getpid());

    hook::Penetrator penerator;
    if (penerator.ResolveAnalysisModule() != hook::HOOK_SUCCESS)
        return;

    //JNIEnv *env;
    //GetJniEnv(&env);

    // TODO: Currently, only simple gadgets for:
    //   StringBuilder.toString()
    //   String.indexOf(Ljava/lang/String;I)
    // are applied. In the long tern, we should design a gadget compiler to build
    // the gadgets for multiple hook points.

    // NOTE: Currently, the gadget naming is long and annoying. It aims to
    // fullfil the idiom of Java method signature. In the future, a more
    // systematic approach will be applied.

    //Gadget_java_lang_StringBuilder gadget_stringbuilder;
    //gadget_stringbuilder.toStringPatcher(env);
    //Gadget_java_lang_String gadget_string;
    //gadget_string.indexOf_javalangString_I_Patcher(env);
}
