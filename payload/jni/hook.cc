#include <iostream>
#include <iomanip>
#include <iosfwd>
#include <fstream>
#include <thread>
#include <future>
#include <unistd.h>

#include "globals.h"
#include "logcat.h"
#include "hook.h"
#include "gadget.h"
#include "signature.h"
#include "mirror/art_method-inl.h"
#include "jni_except-inl.h"


namespace hook {

bool Bootstrap::ResolveInjectorDeliveredSymbols()
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

        // Try to read the core library pathname.
        bool found = true;
        char* path = reinterpret_cast<char*>(addr_bgn);
        while (*path != 0)
            ++path;
        ++path;
        size_t len = strlen(kKeyPathCoreLibrary);
        for (size_t i = 0 ; i < len ; ++i) {
            if (kKeyPathCoreLibrary[i] != path[i]) {
                found = false;
                break;
            }
        }
        if (!found)
            break;
        g_lib_path = path + len + 1;

        // Try to read the analysis module pathname.
        found = true;
        path += len + 1 + strlen(g_lib_path) + 1;
        len = strlen(kKeyPathAnalysisModule);
        for (size_t i = 0 ; i < len ; ++i) {
            if (kKeyPathAnalysisModule[i] != path[i]) {
                found = false;
                break;
            }
        }
        if (!found)
            break;
        g_module_path = path + len + 1;

        // Try to read the main class name of the analysis module.
        found = true;
        path += len + 1 + strlen(g_module_path) + 1;
        len = strlen(kKeyNameMainClass);
        for (size_t i = 0 ; i < len ; ++i) {
            if (kKeyNameMainClass[i] != path[i]) {
                found = false;
                break;
            }
        }
        if (!found)
            break;
        g_class_name = path + len + 1;
        return HOOK_SUCCESS;
    }
    return HOOK_FAILURE;
}

bool Bootstrap::CraftDexPrivatePath()
{
    char buf[kBlahSizeMid];
    sprintf(buf, "/proc/self/cmdline");

    std::ifstream map(buf, std::ifstream::in);
    if (map.good() && !map.eof()) {
        map.getline(buf, kBlahSizeMid);
        size_t len = 2 * (1 + strlen(kDirDexData)) + (1 + strlen(buf)) +
                     (1 + strlen(kDirInstrument)) + 1;
        char* path = new char[len];
        if (!path)
            return HOOK_FAILURE;
        snprintf(path, len, "/%s/%s/%s/%s", kDirDexData, kDirDexData, buf,
                 kDirInstrument);
        dex_path_.reset(path);
        return HOOK_SUCCESS;
    }
    return HOOK_FAILURE;
}

bool Bootstrap::CreateDexPrivateDir()
{
    struct stat stat_buf;
    if (stat(dex_path_.get(), &stat_buf) == 0) {
        // If the private directory is already created, just return now.
        if (S_ISDIR(stat_buf.st_mode))
            return HOOK_SUCCESS;
        if (unlink(dex_path_.get()) != 0)
            return HOOK_FAILURE;
    }
    return (mkdir(dex_path_.get(), kPrivateDexPerm) == 0)?
           HOOK_SUCCESS : HOOK_FAILURE;
}

bool Bootstrap::CacheJVM()
{
    JNIEnv *env;
    // Apply the manually crafted assembly gadget to get the JNIEnv* handle.
    GetJniEnv(&env);
    // Apply the Android JNI to get the JVM handle.
    return (env->GetJavaVM(&g_jvm) == JNI_OK)? HOOK_SUCCESS : HOOK_FAILURE;
}

bool Bootstrap::LoadAnalysisModule()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    char sig[kBlahSizeMid];
    jthrowable except;
    // Resolve "static ClassLoader ClassLoader.getSystemClassLoader()".
    jclass clazz = env->FindClass(kNormClassLoader);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    snprintf(sig, kBlahSizeMid, "()%s", kSigClassLoader);
    jmethodID meth = env->GetStaticMethodID(clazz, kFuncGetSystemClassLoader, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Get "java.lang.ClassLoader".
    jobject class_loader = env->CallStaticObjectMethod(clazz, meth);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Resolve "DexClassLoader.DexClassLoader(String, String, String, ClassLoader)".
    clazz = env->FindClass(kNormDexClassLoader);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    snprintf(sig, kBlahSizeMid, "(%s%s%s%s)V", kSigString, kSigString,
             kSigString, kSigClassLoader);
    meth = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Convert the pathname strings to UTF format.
    jstring path_module = env->NewStringUTF(g_module_path);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    jstring path_cache = env->NewStringUTF(dex_path_.get());
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Create the custom "dalvik.system.DexClassLoader".
    jobject dex_class_loader = env->NewObject(clazz, meth, path_module, path_cache,
                                              path_cache, class_loader);

    // Resolve "Class DexClassLoader.loadClass(String, boolean)".
    snprintf(sig, kBlahSizeMid, "(%s)%s", kSigString, kSigClass);
    meth = env->GetMethodID(clazz, kFuncLoadClass, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Convert the main class of the analysis APK to UTF format.
    jstring name_main = env->NewStringUTF(g_class_name);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Load the main class of the analysis APK.
    g_class_analysis_main = reinterpret_cast<jclass>(env->CallObjectMethod(
                                            dex_class_loader, meth, name_main));
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Resolve the constructor of this main class.
    snprintf(sig, kBlahSizeMid, "()V");
    meth = env->GetMethodID(g_class_analysis_main, kFuncConstructor, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Instantiate an object for it and cache the instance.
    jobject obj_analysis_main = env->NewObject(g_class_analysis_main, meth);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    g_obj_analysis_main = env->NewGlobalRef(obj_analysis_main);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    env->DeleteLocalRef(path_module);
    env->DeleteLocalRef(path_cache);
    env->DeleteLocalRef(name_main);
    g_jvm->DetachCurrentThread();
    return HOOK_SUCCESS;
}

bool Bootstrap::ResolveArtSymbol()
{
    handle_.reset(dlopen(kPathLibArt, RTLD_LAZY));
    if (!handle_.get())
        return HOOK_FAILURE;
    g_indirect_reference_table_add = handle_.resolve(kIndirectReferneceTableAdd);
    if (!g_indirect_reference_table_add)
        return HOOK_FAILURE;
    g_indirect_reference_table_remove = handle_.resolve(kIndirectReferenceTableRemove);
    if (!g_indirect_reference_table_remove)
        return HOOK_FAILURE;
    g_thread_decode_jobject = handle_.resolve(kThreadDecodeJObject);
    if (!g_thread_decode_jobject)
        return HOOK_FAILURE;
    return HOOK_SUCCESS;
}

bool Bootstrap::DeployInstrumentGadgetComposer()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    char sig[kBlahSizeMid];
    jthrowable except;
    // Resolve "Class ClassLoader.loadClass(String)".
    jclass clazz = env->FindClass(kNormClassLoader);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    snprintf(sig, kBlahSizeMid, "(%s)%s", kSigString, kSigClass);
    jmethodID meth = env->GetMethodID(clazz, kFuncLoadClass, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Change the entry point to the quick-compiled code of "loadClass()" to the
    // trampoline of the instrument gadget composer. So when the first component
    // of the instrumented application "android.app.Application" is ready to be
    // loaded, the program flow will be diverted to the composer. At that time,
    // the ClassLoader for the instrumented application is ready, and we can freely
    // pre-load the application defined classes or methods which we interest in,
    // and also instrument the application defined methods or Android/Java APIs.
    art::ArtMethod *art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    uint64_t entry = art::ArtMethod::GetEntryPointFromQuickCompiledCode(art_meth);
    g_load_class_quick_compiled = reinterpret_cast<void*>(entry);
    entry = reinterpret_cast<uint64_t>(ComposeInstrumentGadgetTrampoline);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry);

    return HOOK_SUCCESS;
}

}

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nInstrumentation Bootstrapping, pid = %d\n\n", getpid());

    hook::Bootstrap bootstrap;
    // Generate the pathnames required by Android DexClassLoader to dynamically
    // load our instrumentation module.
    if (bootstrap.ResolveInjectorDeliveredSymbols() != hook::HOOK_SUCCESS)
        return;

    if (bootstrap.CraftDexPrivatePath() != hook::HOOK_SUCCESS)
        return;
    if (bootstrap.CreateDexPrivateDir() != hook::HOOK_SUCCESS)
        return;

    // Retrieve the JVM handle which is necessary for the JNI interaction later.
    if (bootstrap.CacheJVM() != hook::HOOK_SUCCESS)
        return;

    // Load our instrumentation module.
    // To avoid the ANR, we create a worker thread to offload the task bound to
    // the main thread. Note that this is just a work around, more stable
    // approach is needed.
    auto func = std::bind(&hook::Bootstrap::LoadAnalysisModule, &bootstrap);
    std::packaged_task<bool()> task(func);
    std::future<bool> future = task.get_future();
    std::thread thread(std::move(task));
    thread.join();
    if (future.get() != hook::HOOK_SUCCESS)
        return;

    // Resolve the entry points of some critical libart functions which would be
    // used for native code resource management.
    if (bootstrap.ResolveArtSymbol() != hook::HOOK_SUCCESS)
        return;

    // Deploy the hooking gadget composer and finish the bootstrap process.
    // The control flow of the instrumented application will be diverted to
    // the composer when "Class ClassLoader.loadClass(String)" is about to be
    // called to load the first application component "android.app.Application".
    if (bootstrap.DeployInstrumentGadgetComposer() != hook::HOOK_SUCCESS)
        return;
}
