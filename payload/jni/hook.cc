#include <util.h>
#include <bridge.h>

#define LOG_TAG "DEBUG"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)

using namespace util;


// The original functions.
FUNC_INVOKE_VIRTUAL InvokeVirtualOriginal;
FUNC_RESOLVE_STRING ResolveStringOriginal;
FUNC_VIRTUAL_METHOD_CALL VirtualMethodCallOriginal;


void __attribute__((constructor)) entry()
{
    LOGD("\n\n\nHook success, pid = %d\n\n\n", getpid());

    JNIEnv *env;
    GetJNIENVExt(&env);

    JavaVM* vm;
    env->GetJavaVM(&vm);
    art::JavaVMExt* vm_ext = static_cast<art::JavaVMExt*>(vm);
    art::Runtime* runtime = vm_ext->runtime;

    LOGD("\n\n[+] art::Runtime = 0x%08x\n\n", runtime);

    /*
    jclass clazz = env->FindClass("java/lang/Object");
    jmethodID method = env->GetMethodID(clazz, "getClass", "()Ljava/lang/Class;");

    uint32_t addr = (uint32_t)method;
    FUNC_VIRTUAL_METHOD_CALL* addr_quick_entry =
                        (FUNC_VIRTUAL_METHOD_CALL*)(addr + 40);
    VirtualMethodCallOriginal = *addr_quick_entry;
    *addr_quick_entry = VirtualMethodCallFake;

    LOGD("\n\n\n[+] java.lang.Object.getClass() = 0x%08x\n\n\n", method);
    LOGD("\n\n\n[+] VirtualMethodCallOriginal = 0x%08x\n\n\n", VirtualMethodCallOriginal);
    LOGD("\n\n\n[+] VirtualMethodCallFake = 0x%08x\n\n\n", VirtualMethodCallFake);
    */
}


void ResolveStringProfiling(void* referrer, uint32_t string_idx)
{
    /*
    // Now it is just a dummy hook to verify our successful hook.
    LOGD("\n[+] Hooked ResolveString(): ArtMethod=0x%08x StringId=0x%08x\n",
         referrer, string_idx);
    */
}


void VirtualMethodCallProfiling()
{
    /*
    LOGD("\n[+] Execute java.lang.Object.getClass\n");
    */
}

