/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

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
#include "java_type.h"
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

        size_t len_output = 2 * (1 + strlen(kDirDexData)) + (1 + strlen(buf)) + 1;
        char* path_output = new(std::nothrow) char[len_output];
        if (!path_output)
            return PROC_FAIL;

        // The default file output directory for the analysis module.
        size_t len_dex = len_output + (1 + strlen(kDirInstrument));
        char* path_dex = new(std::nothrow) char[len_dex];
        if (!path_dex)
            return PROC_FAIL;

        // The path to store the compiled analysis module.
        snprintf(path_output, len_output, "/%s/%s/%s", kDirDexData, kDirDexData, buf);
        snprintf(path_dex, len_dex, "/%s/%s/%s/%s", kDirDexData, kDirDexData, buf,
                 kDirInstrument);

        output_path_.reset(path_output);
        dex_path_.reset(path_dex);
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
    jsize vm_count;
    GetCreatedJavaVMs(&g_jvm, 1, &vm_count);
    return PROC_SUCC;
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

    // Convert the pathname of the instrumentation package.
    jstring path_module = env->NewStringUTF(g_module_path);
    CHK_EXCP_AND_RET_FAIL(env);

    // Convert the pathname of the directory storing the optimized dex file.
    jstring path_cach_dir = env->NewStringUTF(dex_path_.get());
    CHK_EXCP_AND_RET_FAIL(env);

    // Convert the pathname of the optimized dex file.
    char* idx_bgn = strrchr(g_module_path, '/');
    if (idx_bgn)
        ++idx_bgn;
    else
        idx_bgn = g_module_path;
    char* idx_end = strrchr(g_module_path, '.');
    if (idx_end)
        --idx_end;
    else
        idx_end = g_module_path + strlen(g_module_path) - 1;
    char keyword[kBlahSizeTiny];
    strncpy(keyword, idx_bgn, idx_end - idx_bgn + 1);
    keyword[idx_end - idx_bgn + 1] = 0;

    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "%s/%s.dex", dex_path_.get(), keyword);
    jstring path_cache_file = env->NewStringUTF(sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "static ClassLoader ClassLoader.getSystemClassLoader()".
    jclass clazz = env->FindClass(kNormClassLoader);
    CHK_EXCP_AND_RET_FAIL(env);
    snprintf(sig, kBlahSizeMid, "()%s", kSigClassLoader);
    jmethodID meth = env->GetStaticMethodID(clazz, kFuncGetSystemClassLoader, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Get the system "java.lang.ClassLoader".
    jobject sys_class_loader = env->CallStaticObjectMethod(clazz, meth);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "DexClassLoader.<int>(String, String, String, ClassLoader)".
    clazz = env->FindClass(kNormDexClassLoader);
    CHK_EXCP_AND_RET_FAIL(env);
    snprintf(sig, kBlahSizeMid, "(%s%s%s%s)%c", kSigString, kSigString,
                                kSigString, kSigClassLoader, kSigVoid);
    meth = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Create the "dalvik.system.DexClassLoader" to load the instrumentation package.
    jobject dex_class_loader = env->NewObject(clazz, meth, path_module,
                        path_cach_dir, path_cach_dir, sys_class_loader);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "Class DexClassLoader.loadClass(String)".
    snprintf(sig, kBlahSizeMid, "(%s)%s", kSigString, kSigClass);
    jmethodID meth_load_class = env->GetMethodID(clazz, kFuncLoadClass, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "DexFile.<init>(String)".
    clazz = env->FindClass(kNormDexFile);
    CHK_EXCP_AND_RET_FAIL(env);
    snprintf(sig, kBlahSizeMid, "(%s)%c", kSigString, kSigVoid);
    meth = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Create the "dalvik.system.DexFile" to parse all the defined classes.
    jobject dex_file = env->NewObject(clazz, meth, path_module);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "Enumeration<String> DexFile.entries()".
    snprintf(sig, kBlahSizeMid, "()%s", kSigEnumeration);
    meth = env->GetMethodID(clazz, kFuncEntries, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Get the list of classes defined in the instrumentation package.
    jobject enums = env->CallObjectMethod(dex_file, meth);
    CHK_EXCP_AND_RET_FAIL(env);
    clazz = env->GetObjectClass(enums);

    // Resolve "boolean Enumeration<String>.hasMoreElements()".
    snprintf(sig, kBlahSizeMid, "()%c", kSigBoolean);
    jmethodID meth_has_more = env->GetMethodID(clazz, kFuncHasMoreElements, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    // Resolve "String Enumeration<String>.nextElement()".
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth_next = env->GetMethodID(clazz, kFuncNextElement, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    jclass clazz_instrument;
    while (true) {
        // Enumerate all the classes defined in the instrumentation package.
        jboolean has_next = env->CallBooleanMethod(enums, meth_has_more);
        CHK_EXCP_AND_RET_FAIL(env);
        if (has_next == JNI_FALSE)
            break;

        jobject entry = env->CallObjectMethod(enums, meth_next);
        CHK_EXCP_AND_RET_FAIL(env);

        jstring str_class = reinterpret_cast<jstring>(entry);
        jboolean is_copy = JNI_FALSE;
        const char *cstr_class = env->GetStringUTFChars(str_class, &is_copy);

        // Load the designated class.
        jobject clazz = env->CallObjectMethod(dex_class_loader, meth_load_class, entry);
        if (!clazz) {
            // We can tolerate the failed classes which are defined in
            // "android.support.v*" packages.
            if (strstr(cstr_class, kPrefixAndroidSupport) == nullptr)
                CHK_EXCP_AND_RET_FAIL(env);
            env->ExceptionClear();
        }
        env->DeleteLocalRef(str_class);

        // Keep the class id for "org.probedroid.Instrument".
        if (strcmp(cstr_class, kPkgInstrument) == 0) {
            jobject obj = env->NewLocalRef(clazz);
            CHK_EXCP_AND_RET_FAIL(env);
            clazz_instrument = reinterpret_cast<jclass>(obj);
        }

        if (strcmp(cstr_class, g_class_name) != 0) {
            env->DeleteLocalRef(clazz);
            continue;
        }

        // If the class name matches the main class of the instrumentation package,
        // cache the class and instantiate an object for it.
        g_class_analysis_main = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));
        if (!g_class_analysis_main) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for the main "
                                        "class of the instrument APK.");
            return PROC_FAIL;
        }

        jclass clazz_main = reinterpret_cast<jclass>(clazz);
        snprintf(sig, kBlahSizeMid, "()%c", kSigVoid);
        jmethodID meth_ctor = env->GetMethodID(clazz_main, kFuncConstructor, sig);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject main = env->NewObject(clazz_main, meth_ctor);
        CHK_EXCP_AND_RET_FAIL(env);
        g_obj_analysis_main = env->NewGlobalRef(main);
        if (!g_obj_analysis_main) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for the "
                                       "instantiated main class.");
            return PROC_FAIL;
        }
    }
    // Set the default output folder path for the main instrumentation class.
    jfieldID field_path = env->GetStaticFieldID(clazz_instrument,
                                        kFieldPathOutputDirectory, kSigString);
    CHK_EXCP_AND_RET_FAIL(env);

    jstring path_out = env->NewStringUTF(output_path_.get());
    CHK_EXCP_AND_RET_FAIL(env);
    g_path_output_folder = env->NewGlobalRef(path_out);
    if (!g_path_output_folder) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for the "
                                   "default output folder pathname.");
        return PROC_FAIL;
    }
    env->SetStaticObjectField(clazz_instrument, field_path, g_path_output_folder);

    env->DeleteLocalRef(clazz_instrument);
    env->DeleteLocalRef(dex_class_loader);
    env->DeleteLocalRef(path_cach_dir);
    env->DeleteLocalRef(path_cache_file);
    env->DeleteLocalRef(path_module);
    g_jvm->DetachCurrentThread();

    return PROC_SUCC;
}

bool Bootstrap::ResolveArtSymbol()
{
    handle_.reset(dlopen(kPathLibArt, RTLD_LAZY));
    if (!handle_.get())
        return PROC_FAIL;

    // The function to acquire Java VM handle.
    g_get_created_java_vms = handle_.resolve(kGetCreatedJavaVMs);
    if (!g_get_created_java_vms)
        return PROC_FAIL;

    // The functions to handle local garbage collection.
    g_indirect_reference_table_add = handle_.resolve(kIndirectReferneceTableAdd);
    if (!g_indirect_reference_table_add)
        return PROC_FAIL;
    g_indirect_reference_table_remove = handle_.resolve(kIndirectReferenceTableRemove);
    if (!g_indirect_reference_table_remove)
        return PROC_FAIL;
    g_thread_decode_jobject = handle_.resolve(kThreadDecodeJObject);
    if (!g_thread_decode_jobject)
        return PROC_FAIL;

    // The functions for exception handling.
    g_create_internal_stack_trace = handle_.resolve(kCreateInternalStackTrace);
    if (!g_create_internal_stack_trace)
        return PROC_FAIL;

    return PROC_SUCC;
}

bool Bootstrap::OpenMemoryPermission()
{
    // Remove the write protection for Thread::CreateInternalStackTrace().
    size_t page_size = getpagesize();
    size_t raw_addr = reinterpret_cast<size_t>(g_create_internal_stack_trace);
    void* align_addr = reinterpret_cast<void*>((raw_addr / page_size) * page_size);
    if (mprotect(align_addr, page_size, kArtCodePerm) == -1) {
        CAT(ERROR) << StringPrintf("%s", strerror(errno));
        return PROC_FAIL;
    }

    return PROC_SUCC;
}

bool Bootstrap::DeployInstrumentGadgetComposer()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    // Resolve "Class ClassLoader.loadClass(String)".
    char sig[kBlahSizeMid];
    jclass clazz = env->FindClass(kNormClassLoader);
    CHK_EXCP_AND_RET_FAIL(env);
    snprintf(sig, kBlahSizeMid, "(%s)%s", kSigString, kSigClass);
    jmethodID meth = env->GetMethodID(clazz, kFuncLoadClass, sig);
    CHK_EXCP_AND_RET_FAIL(env);

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

void __attribute__((constructor)) BootEntry()
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

    // Resolve the entry points of some critical libart functions which would be
    // used for native code garbage collection and exception handling.
    if (bootstrap.ResolveArtSymbol() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Resolve libart symbols.");
    if (bootstrap.OpenMemoryPermission() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Change libart access permission.");

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

    // Deploy the hooking gadget composer and finish the bootstrap process.
    // The control flow of the instrumented application will be diverted to
    // the composer when "Class ClassLoader.loadClass(String)" is about to be
    // called to load the first application component "android.app.Application".
    if (bootstrap.DeployInstrumentGadgetComposer() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Deploy instrument gadget composer.");
}
