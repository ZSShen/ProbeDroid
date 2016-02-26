#include <thread>
#include <future>

#include "globals.h"
#include "logcat.h"
#include "hook.h"
#include "gadget.h"
#include "signature.h"
#include "mirror/art_method-inl.h"


namespace hook {

#define CHECK_AND_LOG_EXCEPTION(jvm, env)                                      \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogJNIException(env, except);                                  \
                jvm->DetachCurrentThread();                                    \
                return HOOK_FAILURE;                                           \
            }                                                                  \
        } while (0);                                                           \


// TODO: A more fine-grained approach is needed instead of this simple logd spew.
void Bootstrap::LogJNIException(JNIEnv* env, jthrowable except)
{
    jclass clazz = env->FindClass(kNormObject);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jboolean is_copy = false;
    const char* cstr = env->GetStringUTFChars(str, &is_copy);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    LOGD("%s\n", cstr);
}

bool Bootstrap::CraftAnalysisModulePath()
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

    // Resolve "DexClassLoader.DexClassLoader(String, String, String, ClassLoader)"
    clazz = env->FindClass(kNormDexClassLoader);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    snprintf(sig, kBlahSizeMid, "(%s%s%s%s)V", kSigString, kSigString,
             kSigString, kSigClassLoader);
    meth = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Convert the pathname strings to UTF format.
    jstring path_module = env->NewStringUTF(module_path_);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);
    jstring path_cache = env->NewStringUTF(dex_path_.get());
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    // Create the custom "dalvik.system.DexClassLoader".
    jobject dex_class_loader = env->NewObject(clazz, meth, path_module, path_cache,
                                              path_cache, class_loader);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env);

    env->DeleteLocalRef(path_module);
    env->DeleteLocalRef(path_cache);
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

bool Bootstrap::DeployHookGadgetCompiler()
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

    // Change the entry point to the quick-compiled code of "loadClass()" to our
    // hooking gadget compiler. So when the first component of the instrumented
    // application "android.app.Application" is ready to be loaded, the program
    // flow will be diverted to our compiler. At that time, since the ClassLoader
    // for the instrumented application is ready, we can freely pre-load the
    // application defined classes or methods which we interest in, and also
    // hook the application defined methods or Android/Java APIs.
    art::ArtMethod *art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    uint64_t entry = art::ArtMethod::GetEntryPointFromQuickCompiledCode(art_meth);
    g_load_class_quick_compiled = reinterpret_cast<void*>(entry);
    entry = reinterpret_cast<uint64_t>(CompileHookGadgetTrampoline);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry);

    return HOOK_SUCCESS;
}

}

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nHook Bootstrapping, pid = %d\n\n", getpid());

    hook::Bootstrap bootstrap;
    // Generate the pathnames required by Android DexClassLoader to dynamically
    // load our instrumentation module.
    if (bootstrap.CraftAnalysisModulePath() != hook::HOOK_SUCCESS)
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

    // Deploy the hooking gadget compiler and finish the bootstrap process.
    // The control flow of the instrumented application will be diverted to
    // the compiler when "Class ClassLoader.loadClass(String)" is about to be
    // used to load the first application component "android.app.Application".
    if (bootstrap.DeployHookGadgetCompiler() != hook::HOOK_SUCCESS)
        return;
}
