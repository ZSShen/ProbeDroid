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


void ResolveStringProfiling(void* referrer, uint32_t string_idx)
{
    // Now it is just a dummy hook to verify our successful hook.
    LOGD("\n[+] Hooked ResolveString(): ArtMethod=0x%08x StringId=0x%08x\n",
         referrer, string_idx);
}
