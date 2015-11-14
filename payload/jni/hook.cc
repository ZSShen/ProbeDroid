#include <util.h>
#include <bridge.h>

#define LOG_TAG "DEBUG"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)

using namespace util;


// The original functions.
FUNC_INVOKE_VIRTUAL InvokeVirtualOriginal;
FUNC_RESOLVE_STRING ResolveStringOriginal;


void __attribute__((constructor)) entry()
{
    LOGD("Hook success, pid = %d\n", getpid());

    HookResolveString(&ResolveStringOriginal, ResolveStringFake);
}


void ResolveStringProfiling()
{
    LOGD("\n\n\n[+] Calling ResolveString() during native execution\n\n\n");
}
