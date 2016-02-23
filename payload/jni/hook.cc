#include <thread>
#include <future>

#include "globals.h"
#include "logcat.h"
#include "hook.h"
#include "gadget.h"
#include "signature.h"


namespace hook {

#define CHECK_AND_LOG_EXCEPTION(env, except)                                   \
        do {                                                                   \
            if ((except = env->ExceptionOccurred())) {                         \
                return HOOK_FAILURE;                                           \
            }                                                                  \
        } while (0);                                                           \


// TODO: A more fine-grained approach is needed instead of this simple logd spew.
void Penetrator::LogJNIException(JNIEnv* env, jthrowable except)
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

bool Penetrator::CraftAnalysisModulePath()
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

bool Penetrator::CraftDexPrivatePath()
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

bool Penetrator::CreateDexPrivateDir()
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

bool Penetrator::CacheJVM()
{
    JNIEnv *env;
    // Apply the manually crafted assembly gadget to get the JNIEnv* handle.
    GetJniEnv(&env);
    // Apply the Android JNI to get the JVM handle.
    return (env->GetJavaVM(&jvm_) == JNI_OK)? HOOK_SUCCESS : HOOK_FAILURE;
}

bool Penetrator::LoadAnalysisModule()
{
    JNIEnv* env;
    jvm_->AttachCurrentThread(&env, nullptr);

    char sig[kBlahSizeMid];
    jthrowable except;
    // Resolve "static ClassLoader ClassLoader.getSystemClassLoader()".
    jclass clazz = env->FindClass(kNormClassLoader);
    CHECK_AND_LOG_EXCEPTION(env, except);
    snprintf(sig, kBlahSizeMid, "()%s", kSigClassLoader);
    jmethodID meth = env->GetStaticMethodID(clazz, kFuncGetSystemClassLoader, sig);
    CHECK_AND_LOG_EXCEPTION(env, except);

    // Get "java.lang.ClassLoader".
    jobject class_loader = env->CallStaticObjectMethod(clazz, meth);
    CHECK_AND_LOG_EXCEPTION(env, except);

    // Resolve "DexClassLoader.DexClassLoader(String, String, String, ClassLoader)"
    clazz = env->FindClass(kNormDexClassLoader);
    CHECK_AND_LOG_EXCEPTION(env, except);
    snprintf(sig, kBlahSizeMid, "(%s%s%s%s)V", kSigString, kSigString,
             kSigString, kSigClassLoader);
    meth = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHECK_AND_LOG_EXCEPTION(env, except);

    // Convert the pathname strings to UTF format.
    jstring path_module = env->NewStringUTF(module_path_);
    CHECK_AND_LOG_EXCEPTION(env, except);
    jstring path_cache = env->NewStringUTF(dex_path_.get());
    CHECK_AND_LOG_EXCEPTION(env, except);

    // Create the custom "dalvik.system.DexClassLoader".
    jobject dex_class_loader = env->NewObject(clazz, meth, path_module, path_cache,
                                              path_cache, class_loader);
    CHECK_AND_LOG_EXCEPTION(env, except);

    jvm_->DetachCurrentThread();
    return HOOK_SUCCESS;
}

}

void __attribute__((constructor)) HookEntry()
{
    LOGD("\n\nHook success, pid = %d\n\n", getpid());

    hook::Penetrator penetrator;
    // Generate the pathnames required by Android DexClassLoader to dynamically
    // load our instrumentation module.
    if (penetrator.CraftAnalysisModulePath() != hook::HOOK_SUCCESS)
        return;
    if (penetrator.CraftDexPrivatePath() != hook::HOOK_SUCCESS)
        return;
    if (penetrator.CreateDexPrivateDir() != hook::HOOK_SUCCESS)
        return;

    // Retrieve the JVM handle which is necessary for the JNI interaction later.
    if (penetrator.CacheJVM() != hook::HOOK_SUCCESS)
        return;

    // Load our instrumentation module.
    // To avoid the ANR, we create a worker thread to offload the task bound to
    // the main thread. Note that this is just a work around, more stable
    // approach is needed.
    auto func = std::bind(&hook::Penetrator::LoadAnalysisModule, &penetrator);
    std::packaged_task<bool()> task(func);
    std::future<bool> future = task.get_future();
    std::thread thread(std::move(task));
    thread.join();
    if (future.get() != hook::HOOK_SUCCESS)
        return;
}
