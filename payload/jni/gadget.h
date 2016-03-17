#ifndef _GADGET_H_
#define _GADGET_H_


#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <jni.h>

#include "signature.h"
#include "logcat.h"


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
      uint64_t quick_code_entry_origin, jobject bundle, jmethodID meth_before_exec,
      jmethodID meth_after_exec)
     : is_static_(is_static),
       type_output_(type_output),
       unboxed_input_width_(0),
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
            if ((type == kTypeLong) || (type == kTypeDouble))
                unboxed_input_width_ += kWidthQword;
            else
                unboxed_input_width_ += kWidthDword;
        }
        if (!is_static_)
            unboxed_input_width_ += kWidthDword;
    }

    ~MethodBundleNative();

    std::mutex& GetMutex()
    {
        return mutex_;
    }

    int32_t GetUnboxedInputWidth()
    {
        return unboxed_input_width_;
    }

    const std::vector<char>& GetInputTypes()
    {
        return type_inputs_;
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

  private:
    bool is_static_;
    char type_output_;
    int32_t unboxed_input_width_;
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

class PrimitiveTypeWrapper
{
  public:
    PrimitiveTypeWrapper(jclass clazz, jmethodID meth_ctor, jmethodID meth_access)
     : clazz_(clazz),
       meth_ctor_(meth_ctor),
       meth_access_(meth_access)
    {}

    ~PrimitiveTypeWrapper();

    jclass GetClass()
    {
        return clazz_;
    }

    jmethodID GetConstructor()
    {
        return meth_ctor_;
    }

    jmethodID GetAccessor()
    {
        return meth_access_;
    }

    static bool LoadWrappers(JNIEnv*);

  private:
    jclass clazz_;
    jmethodID meth_ctor_;
    jmethodID meth_access_;
};

class ClassCache
{
  public:
    ClassCache(jclass clazz)
     : clazz_(clazz),
       map_meth_()
    {}

    ~ClassCache();

    jclass GetClass()
    {
        return clazz_;
    }

    void CacheMethod(const std::string& signature, jmethodID meth)
    {
        map_meth_.insert(std::make_pair(signature, meth));
    }

    jmethodID GetCachedMethod(const std::string& signature)
    {
        auto iter = map_meth_.find(signature);
        return (iter != map_meth_.end())? iter->second : 0;
    }

    static bool LoadClasses(JNIEnv*);

  private:
    jclass clazz_;
    std::unordered_map<std::string, jmethodID> map_meth_;
};


// The gadget to extract JNI handle from TLS.
extern "C" void GetJniEnv(JNIEnv**) __asm__("GetJniEnv");

// The gadget to insert an object into the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" jobject AddIndirectReference(void*, uint32_t, void*)
                                        __asm__("AddIndirectReference");

// The gadget to remove a reference from the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" bool RemoveIndirectReference(void*, uint32_t, jobject)
                                        __asm__("RemoveIndirectReference");

// The gadget to decode the given indirect reference.
// Note that the first argument is the pointer to art::Thread.
extern "C" void* DecodeJObject(void*, jobject) __asm__("DecodeJObject");

// The trampoline to the function to set instrument gadget composer.
extern "C" void* ComposeInstrumentGadgetTrampoline()
                                        __asm__("ComposeInstrumentGadgetTrampoline");

// The trampoline to the function to marshall the instrument callbacks and the
// original method call.
extern "C" void* ArtQuickInstrumentTrampoline()
                                        __asm__("ArtQuickInstrumentTrampoline");

// The function which launches the composer that will set all the instrument
// gadgets towards user designated Java methods for instrumentation.
extern "C" void* ComposeInstrumentGadget(void*, void*, void*, void*, void*);

// The function which marshalls the callbacks including the before and after method
// execution calls for instrumentation. Also, it will invoke the original method
// to fulfill the expected behavior of instrumented app.
extern "C" void ArtQuickInstrument(void**, void**, void*, void*, void*, void*, void**);


// The cached symbols delivered from injector.
extern char* g_module_path;
extern char* g_lib_path;
extern char* g_class_name;

// The cached Java VM handle.
extern JavaVM* g_jvm;

// The original entry to IndirectReferenceTable::Add().
extern void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
extern void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
extern void* g_thread_decode_jobject;

// The original entry to the loadClass() quick compiled code.
extern void* g_load_class_quick_compiled;

// The cached class and object instance of analysis module.
extern jclass g_class_analysis_main;
extern jobject g_obj_analysis_main;

// The global map to maintain the information about all the instrumented methods
// of the target app.
typedef std::unique_ptr<std::unordered_map<jmethodID, std::unique_ptr<MethodBundleNative>>>
        PtrBundleMap;
extern PtrBundleMap g_map_method_bundle;

// The global map to cache the access information about all the wrappers of
// primitive Java types.
typedef std::unique_ptr<std::unordered_map<char, std::unique_ptr<PrimitiveTypeWrapper>>>
        PtrPrimitiveMap;
extern PtrPrimitiveMap g_map_primitive_wrapper;

// The global map to cache the frequently used classes and method ids.
typedef std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<ClassCache>>>
        PtrClassMap;
extern PtrClassMap g_map_class_cache;

#endif