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

#ifndef _GADGET_H_
#define _GADGET_H_


#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <thread>
#include <jni.h>
#include <setjmp.h>

#include "globals.h"
#include "signature.h"
#include "logcat.h"
#if defined(__arm__)
    #include "gadget_arm.h"
#else
    #include "gadget_x86.h"
#endif
#include "ffi.h"


class InstrumentGadgetComposer
{
  public:
    InstrumentGadgetComposer(JNIEnv *env, jobject ref_class_loader,
                             jmethodID id_load_class)
      : env_(env),
        ref_class_loader_(ref_class_loader),
        id_load_class_(id_load_class)
    {}

    void Compose();
    static void UnregisterInstrumentGadget(int32_t);

  private:
    bool LinkWithAnalysisAPK();
    bool RegisterInstrumentGadget();

    JNIEnv* env_;
    jobject ref_class_loader_;
    jmethodID id_load_class_;
};

class MethodBundleNative
{
  public:
    MethodBundleNative(bool is_static, const char* name_class, const char* name_method,
      const char* signature_method, const std::vector<char>& type_inputs, char type_output,
      jclass clazz, uint64_t quick_code_entry_origin, jobject bundle,
      jmethodID meth_before_exec, jmethodID meth_after_exec)
     : is_static_(is_static),
       type_output_(type_output),
       input_width_(0),
       clazz_(clazz),
       bundle_(bundle),
       meth_before_exec_(meth_before_exec),
       meth_after_exec_(meth_after_exec),
       quick_code_entry_origin_(quick_code_entry_origin),
       name_class_(name_class),
       name_method_(name_method),
       signature_method_(signature_method),
       type_inputs_(type_inputs),
       mutex_()
    {
        for (char type : type_inputs_) {
            if ((type == kTypeLong) || (type == kTypeFloat) || (type == kTypeDouble))
                input_width_ += kWidthQword;
            else
                input_width_ += kWidthDword;
        }
    }

    ~MethodBundleNative();

    std::mutex& GetMutex()
    {
        return mutex_;
    }

    int32_t GetInputWidth()
    {
        return input_width_;
    }

    bool IsStatic()
    {
        return is_static_;
    }

    const std::vector<char>& GetInputType()
    {
        return type_inputs_;
    }

    char GetOutputType()
    {
        return type_output_;
    }

    jobject GetBundleObject()
    {
        return bundle_;
    }

    jmethodID GetBeforeExecuteCallback()
    {
        return meth_before_exec_;
    }

    jmethodID GetAfterExecuteCallback()
    {
        return meth_after_exec_;
    }

    uint64_t GetQuickCodeOriginalEntry()
    {
        return quick_code_entry_origin_;
    }

    jclass GetClass()
    {
        return clazz_;
    }

    const std::string& GetClassName()
    {
        return name_class_;
    }

    const std::string& GetMethodName()
    {
        return name_method_;
    }

    const std::string& GetMethodSignature()
    {
        return signature_method_;
    }

  private:
    bool is_static_;
    char type_output_;
    int32_t input_width_;
    jclass clazz_;
    jobject bundle_;
    jmethodID meth_before_exec_;
    jmethodID meth_after_exec_;
    uint64_t quick_code_entry_origin_;
    std::string name_class_;
    std::string name_method_;
    std::string signature_method_;
    std::vector<char> type_inputs_;
    std::mutex mutex_;
};

class MarshallingYard
{
  public:
    MarshallingYard(JNIEnv *env, MethodBundleNative* bundle_native,
                    InputMarshaller& input_marshaller,
                    OutputMarshaller& output_marshaller)
      : env_(reinterpret_cast<JNIEnvExt*>(env)),
        bundle_native_(bundle_native),
        input_marshaller_(input_marshaller),
        output_marshaller_(output_marshaller),
        gc_auto_(),
        gc_manual_()
    {
        // Resolve some important members of JNIEnvExt for resource management.
        cookie_ = env_->local_ref_cookie_;
        ref_table_ = reinterpret_cast<IndirectReferenceTable*>(&(env_->local_refs_table_));
        thread_ = env_->thread_;
    }

    void Launch();

  private:
    bool BoxInput(jobjectArray, void**, const std::vector<char>&);
    bool UnboxInput(jobjectArray, void**, const std::vector<char>&);
    bool BoxOutput(jobject*, void**, char);
    bool UnboxOutput(jobject, void**, char);
    bool EncapsulateObject(char, void***, jobject*);
    bool DecapsulateObject(char, bool, void***, jobject);

    void MakeGenericInput(void**, const std::vector<char>&,
                          jobject*, jclass*, jmethodID*, ffi_type**, void**);
    bool InvokeOrigin(int32_t, jmethodID, char, ffi_type**, void**, void**);


    static constexpr const int32_t kMinJniArgCount = 3;

    uint32_t cookie_;
    JNIEnvExt* env_;
    IndirectReferenceTable* ref_table_;
    void* thread_;

    MethodBundleNative* bundle_native_;
    InputMarshaller& input_marshaller_;
    OutputMarshaller& output_marshaller_;

    std::vector<jobject> gc_auto_;
    std::vector<jobject> gc_manual_;
};


// The gadget to acquire the Java VM handle.
extern "C" jint GetCreatedJavaVMs(JavaVM**, jsize, jsize*);

// The gadget to insert an object into the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" jobject AddIndirectReference(IndirectReferenceTable*, uint32_t, void*);

// The gadget to remove a reference from the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" bool RemoveIndirectReference(IndirectReferenceTable*, uint32_t, jobject);

// The gadget to decode the given indirect reference.
// Note that the first argument is the pointer to art::Thread.
extern "C" void* DecodeJObject(void*, jobject);

// The gadget to silence the Runtime stack trace for exception object creation.
extern "C" void CloseRuntimeStackTrace();

// The gadget to restore the Runtime stack trace for exception object creation.
extern "C" void OpenRuntimeStackTrace();

// The gadget to extract the function pointer to art_quick_deliver_exception.
extern "C" void GetFuncDeliverException(void**) __asm__("GetFuncDeliverException");

// The gadget to replace the function pointer to art_quick_deliver_exception.
extern "C" void SetFuncDeliverException(void*) __asm__("SetFuncDeliverException");

// The trampoline to the function to set instrument gadget composer.
extern "C" void* ComposeInstrumentGadgetTrampoline()
                                        __asm__("ComposeInstrumentGadgetTrampoline");

// The trampoline to the function to marshall the instrument callbacks and the
// original method call.
extern "C" void* ArtQuickInstrumentTrampoline()
                                        __asm__("ArtQuickInstrumentTrampoline");

// The trampoline to the fake exception delivery function which is used to
// detour the Java exception thrown to ProbeDroid native.
extern "C" void* ArtQuickDeliverExceptionTrampoline()
                                    __asm__("ArtQuickDeliverExceptionTrampoline");

// The function which launches the composer that will set all the instrument
// gadgets towards user designated Java methods for instrumentation.
extern "C" void* ComposeInstrumentGadget(void*, void*, void*);

// The function which marshalls the callbacks including the before and after method
// execution calls for instrumentation. Also, it will invoke the original method
// to fulfill the expected behavior of instrumented app.
extern "C" void ArtQuickInstrument(void**, void**, void*, void*, void*, void*, void**);

// The fake exception delivery function which is used to detour the Java exception
// thrown to ProbeDroid native.
extern "C" void ArtQuickDeliverException(void*);

// The cached symbols delivered from injector.
extern char* g_module_path;
extern char* g_lib_path;
extern char* g_class_name;

// The cached Java VM handle.
extern JavaVM* g_jvm;

// The original entry to JNI_GetCreatedJavaVMs.
extern void* g_get_created_java_vms;

// The original entry to IndirectReferenceTable::Add().
extern void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
extern void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
extern void* g_thread_decode_jobject;

// The original entry to Thread::CreateInternalStackTrace().
extern void* g_create_internal_stack_trace;

// The original entry to the loadClass() quick compiled code.
extern void* g_load_class_quick_compiled;

// The cached class and object instance of analysis module.
extern jclass g_class_analysis_main;
extern jobject g_obj_analysis_main;
extern jobject g_path_output_folder;

// The cached class loader object and method to load the classes defined in the
// to be instrumented APK.
extern jobject g_ref_class_loader;
extern jmethodID g_meth_load_class;

// The reentrant counter to avoid hook loop.
extern thread_local uint32_t g_entrant_count;

// The buffer to cache the prologue of Thread::CreateInternalStackTrace.
#if defined(__arm__)
    extern uint8_t g_prologue_original_stack_trace[kCacheSizeQWord];
    extern uint8_t g_prologue_hooked_stack_trace[kCacheSizeQWord];
#else
    extern uint8_t g_prologue_original_stack_trace[kCacheSizeDWord];
    extern uint8_t g_prologue_hooked_stack_trace[kCacheSizeDWord];
#endif

// The check point for exception restore when ProbeDroid native invoke JNI
// function which may fail and throw exception.
extern jmp_buf g_save_ptr;

// The global map to maintain the information about all the instrumented methods
// of the target app.
using PtrBundleMap =
std::unique_ptr<std::unordered_map<jmethodID, std::unique_ptr<MethodBundleNative>>>;
extern PtrBundleMap g_map_method_bundle;


#endif