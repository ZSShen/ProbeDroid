#include <iostream>
#include <iomanip>
#include <iosfwd>
#include <fstream>
#include <cstring>
#include <new>
#include <thread>
#include <future>
#include <unistd.h>

#include "boot.h"
#include "globals.h"
#include "stringprintf.h"
#include "logcat.h"
#include "gadget.h"
#include "signature.h"
#include "mirror/art_method-inl.h"
#include "jni_except-inl.h"


namespace boot {

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
        return PROC_SUCC;
    }
    return PROC_FAIL;
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
        char* path = new(std::nothrow) char[len];
        if (!path)
            return PROC_FAIL;
        snprintf(path, len, "/%s/%s/%s/%s", kDirDexData, kDirDexData, buf,
                 kDirInstrument);
        dex_path_.reset(path);
        return PROC_SUCC;
    }
    return PROC_FAIL;
}

bool Bootstrap::CreateDexPrivateDir()
{
    struct stat stat_buf;
    if (stat(dex_path_.get(), &stat_buf) == 0) {
        // If the private directory is already created, just return now.
        if (S_ISDIR(stat_buf.st_mode))
            return PROC_SUCC;
        if (unlink(dex_path_.get()) != 0)
            return PROC_FAIL;
    }
    return (mkdir(dex_path_.get(), kPrivateDexPerm) == 0)?
           PROC_SUCC : PROC_FAIL;
}

bool Bootstrap::CacheJVM()
{
    JNIEnv *env;
    // Apply the manually crafted assembly gadget to get the JNIEnv* handle.
    GetJniEnv(&env);
    // Apply the Android JNI to get the JVM handle.
    return (env->GetJavaVM(&g_jvm) == JNI_OK)? PROC_SUCC : PROC_FAIL;
}

bool Bootstrap::CacheHotJavaTypes()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    typedef std::unordered_map<char, std::unique_ptr<PrimitiveTypeWrapper>>
            PrimitiveMap;
    PrimitiveMap* primitive_map = new(std::nothrow)PrimitiveMap();
    if (!primitive_map) {
        CAT(ERROR) << StringPrintf("Allocate global map for PrimitiveTypeWrapper.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper.reset(primitive_map);

    typedef std::unordered_map<std::string, std::unique_ptr<ClassCache>>
            ClassMap;
    ClassMap* class_map = new(std::nothrow)ClassMap();
    if (!class_map) {
        CAT(ERROR) << StringPrintf("Allocate global map for ClassCache.");
        return PROC_FAIL;
    }
    g_map_class_cache.reset(class_map);

    if (PrimitiveTypeWrapper::LoadWrappers(env) != PROC_SUCC)
        return PROC_FAIL;
    return ClassCache::LoadClasses(env);
}

bool Bootstrap::LoadAnalysisModule()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    char sig[kBlahSizeMid];
    // Resolve "static ClassLoader ClassLoader.getSystemClassLoader()".
    jclass clazz = env->FindClass(kNormClassLoader);
    snprintf(sig, kBlahSizeMid, "()%s", kSigClassLoader);
    jmethodID meth = env->GetStaticMethodID(clazz, kFuncGetSystemClassLoader, sig);

    // Get the system "java.lang.ClassLoader".
    jobject class_loader = env->CallStaticObjectMethod(clazz, meth);
    CHK_EXCP_AND_DETACH(g_jvm, env);

    // Convert the pathname strings to UTF format.
    jstring path_module = env->NewStringUTF(g_module_path);
    snprintf(sig, kBlahSizeMid, "%s/%s", dex_path_.get(), kDexFileTemp);
    jstring path_cache = env->NewStringUTF(sig);

    // Resolve "static DexFile DexFile.loadDex(String, String, int)".
    jclass clazz_dexfile = env->FindClass(kNormDexFile);
    snprintf(sig, kBlahSizeMid, "(%s%s%c)%s", kSigString, kSigString, kSigInt,
                                              kSigDexFile);
    meth = env->GetStaticMethodID(clazz_dexfile, kFuncLoadDex, sig);

    // Load the optimized dex file of the analysis APK.
    jobject dexfile = env->CallStaticObjectMethod(clazz_dexfile, meth, path_module,
                                                   path_cache, 0);
    CHK_EXCP_AND_DETACH(g_jvm, env);

    // Resolve "Enumeration<String> DexFile.entries()".
    snprintf(sig, kBlahSizeMid, "()%s", kSigEnumeration);
    meth = env->GetMethodID(clazz_dexfile, kFuncEntries, sig);

    // Get the list of classes defined in the analysis APK.
    jobject enums = env->CallObjectMethod(dexfile, meth);
    CHK_EXCP_AND_DETACH(g_jvm, env);
    jclass clazz_enums = env->GetObjectClass(enums);

    // Resolve "boolean Enumeration<String>.hasMoreElements()".
    snprintf(sig, kBlahSizeMid, "()%c", kSigBoolean);
    jmethodID meth_has_more = env->GetMethodID(clazz_enums, kFuncHasMoreElements, sig);

    // Resolve "String Enumeration<String>.nextElement()".
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth_next = env->GetMethodID(clazz_enums, kFuncNextElement, sig);

    // Resolve "Class DexFile.loadClass(String, ClassLoader)".
    snprintf(sig, kBlahSizeMid, "(%s%s)%s", kSigString, kSigClassLoader, kSigClass);
    meth = env->GetMethodID(clazz_dexfile, kFuncLoadClass, sig);

    while (true) {
        // Try to load all the classes defined in the analysis APK.
        jboolean has_next = env->CallBooleanMethod(enums, meth_has_more);
        CHK_EXCP_AND_DETACH(g_jvm, env);
        if (has_next == JNI_FALSE)
            break;

        jobject entry = env->CallObjectMethod(enums, meth_next);
        CHK_EXCP_AND_DETACH(g_jvm, env);
        jstring str_class = reinterpret_cast<jstring>(entry);
        jboolean is_copy = JNI_FALSE;
        const char *cstr_class = env->GetStringUTFChars(str_class, &is_copy);

        jobject clazz = env->CallObjectMethod(dexfile, meth, entry, class_loader);
        CHK_EXCP_AND_DETACH(g_jvm, env);
        if (strcmp(cstr_class, g_class_name) != 0)
            continue;

        // If the class name matches the main class of the analysis APK, cache
        // the class and instantiate an object for it.
        g_class_analysis_main = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));

        jclass clazz_main = reinterpret_cast<jclass>(clazz);
        snprintf(sig, kBlahSizeMid, "()%c", kSigVoid);
        jmethodID meth_ctor = env->GetMethodID(clazz_main, kFuncConstructor, sig);
        CHK_EXCP_AND_DETACH(g_jvm, env);

        jobject main = env->NewObject(clazz_main, meth_ctor);
        g_obj_analysis_main = env->NewGlobalRef(main);
    }

    env->DeleteLocalRef(path_module);
    env->DeleteLocalRef(path_cache);
    g_jvm->DetachCurrentThread();
    return PROC_SUCC;
}

bool Bootstrap::ResolveArtSymbol()
{
    handle_.reset(dlopen(kPathLibArt, RTLD_LAZY));
    if (!handle_.get())
        return PROC_FAIL;
    g_indirect_reference_table_add = handle_.resolve(kIndirectReferneceTableAdd);
    if (!g_indirect_reference_table_add)
        return PROC_FAIL;
    g_indirect_reference_table_remove = handle_.resolve(kIndirectReferenceTableRemove);
    if (!g_indirect_reference_table_remove)
        return PROC_FAIL;
    g_thread_decode_jobject = handle_.resolve(kThreadDecodeJObject);
    if (!g_thread_decode_jobject)
        return PROC_FAIL;
    return PROC_SUCC;
}

bool Bootstrap::DeployInstrumentGadgetComposer()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    char sig[kBlahSizeMid];
    jthrowable except;
    // Resolve "Class ClassLoader.loadClass(String)".
    jclass clazz = env->FindClass(kNormClassLoader);
    snprintf(sig, kBlahSizeMid, "(%s)%s", kSigString, kSigClass);
    jmethodID meth = env->GetMethodID(clazz, kFuncLoadClass, sig);

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

    return PROC_SUCC;
}

}

void __attribute__((constructor)) HookEntry()
{
    CAT(INFO) << StringPrintf("Instrument Bootstrap, pid=%d\n", getpid());

    boot::Bootstrap bootstrap;
    // Generate the pathnames required by Android DexClassLoader to dynamically
    // load our instrument module.
    if (bootstrap.ResolveInjectorDeliveredSymbols() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Resolve module paths.");

    if (bootstrap.CraftDexPrivatePath() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Craft installation path for instrument APK.");

    if (bootstrap.CreateDexPrivateDir() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Create APK installation directory.");

    // Retrieve the JVM handle which is necessary for the JNI interaction later.
    if (bootstrap.CacheJVM() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Cache JVM handle.");

    // Cache the access information about some hot Java types.
    if (bootstrap.CacheHotJavaTypes() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Preload hot Java types.");

    // Load our instrumentation module.
    // To avoid the ANR, we create a worker thread to offload the task bound to
    // the main thread. Note that this is just a work around, more stable
    // approach is needed.
    auto func = std::bind(&boot::Bootstrap::LoadAnalysisModule, &bootstrap);
    std::packaged_task<bool()> task(func);
    std::future<bool> future = task.get_future();
    std::thread thread(std::move(task));
    thread.join();
    if (future.get() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Install and load instrument APK.");

    // Resolve the entry points of some critical libart functions which would be
    // used for native code resource management.
    if (bootstrap.ResolveArtSymbol() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Resolve libart symbols.");

    // Deploy the hooking gadget composer and finish the bootstrap process.
    // The control flow of the instrumented application will be diverted to
    // the composer when "Class ClassLoader.loadClass(String)" is about to be
    // called to load the first application component "android.app.Application".
    if (bootstrap.DeployInstrumentGadgetComposer() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Deploy instrument gadget composer.");
}
