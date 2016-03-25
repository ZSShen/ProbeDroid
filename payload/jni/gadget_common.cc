#include <cstdlib>
#include <cstring>
#include <mutex>
#include <new>
#include <signal.h>
#include <errno.h>

#include "gadget.h"
#include "jni_except-inl.h"
#include "mirror/art_method-inl.h"
#include "stringprintf.h"
#include "ffi.h"


// The cached symbols delivered from injector.
char* g_module_path;
char* g_lib_path;
char* g_class_name;

// The cached Java VM handle.
JavaVM* g_jvm;

// The original entry to IndirectReferenceTable::Add().
void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
void* g_thread_decode_jobject;

// The original entry to the loadClass() quick compiled code.
void* g_load_class_quick_compiled;

// The cached class and object instance of analysis module.
jclass g_class_analysis_main;
jobject g_obj_analysis_main;

// The cached class loader object and method to load the classes defined in the
// to be instrumented APK.
jobject g_ref_class_loader;
jmethodID g_meth_load_class;

// The global map to maintain the information about all the instrumented methods
// of the target app.
PtrBundleMap g_map_method_bundle(nullptr);

// The global map to cache the access information about all the wrappers of
// primitive Java types.
PtrPrimitiveMap g_map_primitive_wrapper(nullptr);

// The global map to cache the frequently used method ids.
PtrClassMap g_map_class_cache(nullptr);


void InstrumentGadgetComposer::Compose()
{
    if (LinkWithAnalysisAPK() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Link instrument APK.");

    // Initialize the global map for method information maintenance.
    typedef std::unordered_map<jmethodID, std::unique_ptr<MethodBundleNative>>
            BundleMap;
    BundleMap* bundle_map = new(std::nothrow)BundleMap();
    if (!bundle_map)
        CAT(FATAL) << StringPrintf("Allocate map for MethodBundleNative.");
    g_map_method_bundle.reset(bundle_map);

    // Invoke the overrode "void Instrument.onApplicationStart()" to let the
    // analysis APK register the methods which it intends to instrument.
    if (RegisterInstrumentGadget() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Call Instrument.onApplicationStart().");

    // Register the SIGTERM signal handler to catch the instrument termination
    // command sent by analyst. The handler further calls
    // "void Instrument.onApplicationStop()" to let analysis APK finalize
    // the instrument task.
    if (signal(SIGTERM, UnregisterInstrumentGadget) == SIG_ERR)
        CAT(FATAL) << StringPrintf("%s", strerror(errno));
}

bool InstrumentGadgetComposer::LinkWithAnalysisAPK()
{
    // Resolve "void Instrument.prepareNativeBridge(String)".
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "(%s)%c", kSigString, kSigVoid);
    jmethodID meth = env_->GetMethodID(g_class_analysis_main,
                                       kFuncPrepareNativeBridge, sig);
    CHK_EXCP_AND_RET_FAIL(env_);

    // Convert the library pathname to UTF format.
    jstring path_module = env_->NewStringUTF(g_lib_path);
    CHK_EXCP_AND_RET_FAIL(env_);

    // Invoke it to let the analysis APK link with this native library.
    env_->CallVoidMethod(g_obj_analysis_main, meth, path_module);
    CHK_EXCP_AND_RET_FAIL(env_);

    env_->DeleteLocalRef(path_module);
    return PROC_SUCC;
}

bool InstrumentGadgetComposer::RegisterInstrumentGadget()
{
    // Resolve the overrode "void Instrument.onApplicationStart()".
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%c", kSigVoid);
    jmethodID meth = env_->GetMethodID(g_class_analysis_main,
                                       kFuncOnApplicationStart, sig);
    CHK_EXCP_AND_RET_FAIL(env_);

    // Invoke it to let the analysis APK deploy the instrument gadget.
    env_->CallVoidMethod(g_obj_analysis_main, meth);
    CHK_EXCP_AND_RET_FAIL(env_);

    return PROC_SUCC;
}

void InstrumentGadgetComposer::UnregisterInstrumentGadget(int32_t signum)
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    // Resolve the overrode "void Instrument.onApplicationStop()".
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%c", kSigVoid);
    jmethodID meth = env->GetMethodID(g_class_analysis_main,
                                      kFuncOnApplicationStop, sig);
    CHK_EXCP(env, exit(EXIT_FAILURE));

    // Invoke it to let the analysis APK finalize the instrument task.
    env->CallVoidMethod(g_obj_analysis_main, meth);
    CHK_EXCP(env, exit(EXIT_FAILURE));

    exit(EXIT_SUCCESS);
}

MethodBundleNative::~MethodBundleNative()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(clazz_);
    env->DeleteGlobalRef(bundle_);
}

PrimitiveTypeWrapper::~PrimitiveTypeWrapper()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(clazz_);
}

bool PrimitiveTypeWrapper::LoadWrappers(JNIEnv* env)
{
    // Load Boolean wrapper.
    char sig[kBlahSizeMid];
    jclass clazz = env->FindClass(kNormBooleanObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigBoolean, kSigVoid);
    jmethodID meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigBoolean);
    jmethodID meth_access = env->GetMethodID(clazz, kFuncBooleanValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Boolean class.");
        return PROC_FAIL;
    }
    jclass g_clazz = reinterpret_cast<jclass>(g_ref);
    PrimitiveTypeWrapper* wrapper = new(std::nothrow) PrimitiveTypeWrapper(
                                        g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Boolean.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeBoolean,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Byte Wrapper.
    clazz = env->FindClass(kNormByteObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigByte, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigByte);
    meth_access = env->GetMethodID(clazz, kFuncByteValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Byte class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Byte.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeByte,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Character Wrapper.
    clazz = env->FindClass(kNormCharObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigChar, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigChar);
    meth_access = env->GetMethodID(clazz, kFuncCharValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Character class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Character.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeChar,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Short Wrapper.
    clazz = env->FindClass(kNormShortObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigShort, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigShort);
    meth_access = env->GetMethodID(clazz, kFuncShortValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Short class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Short.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeShort,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Integer Wrapper.
    clazz = env->FindClass(kNormIntObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigInt, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigInt);
    meth_access = env->GetMethodID(clazz, kFuncIntValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Integer class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Integer.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeInt,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Float Wrapper.
    clazz = env->FindClass(kNormFloatObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigFloat, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigFloat);
    meth_access = env->GetMethodID(clazz, kFuncFloatValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Float class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Float.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeFloat,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Long Wrapper.
    clazz = env->FindClass(kNormLongObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigLong, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigLong);
    meth_access = env->GetMethodID(clazz, kFuncLongValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Long class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Long.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeLong,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Double Wrapper.
    clazz = env->FindClass(kNormDoubleObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigDouble, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigDouble);
    meth_access = env->GetMethodID(clazz, kFuncDoubleValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Double class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Double.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeDouble,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    return PROC_SUCC;
}

ClassCache::~ClassCache()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(clazz_);
}

bool ClassCache::LoadClasses(JNIEnv* env)
{
    char sig[kBlahSizeMid];
    // Load "java.lang.Object".
    {
        jclass clazz = env->FindClass(kNormObject);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for Object class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for Object.");
            return PROC_FAIL;
        }

        // Load "String Object.toString()".
        {
            snprintf(sig, kBlahSizeMid, "()%s", kSigString);
            jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
            CHK_EXCP_AND_RET_FAIL(env);

            snprintf(sig, kBlahSizeMid, "%s()%s", kFuncToString, kSigString);
            std::string sig_method(sig);
            class_cache->CacheMethod(sig_method, meth);
        }
        std::string sig_class(kNormObject);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.IllegalArgumentException".
    {
        jclass clazz = env->FindClass(kNormIllegalArgument);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "IllegalArgumentException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for IllegalArgumentException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormIllegalArgument);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.ClassNotFoundException".
    {
        jclass clazz = env->FindClass(kNormClassNotFound);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "ClassNotFoundException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for ClassNotFoundException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormClassNotFound);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.NoSuchMethodException".
    {
        jclass clazz = env->FindClass(kNormNoSuchMethod);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "NoSuchMethodException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow)ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for NoSuchMethodException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormNoSuchMethod);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    return PROC_SUCC;
}

void MarshallingYard::Launch()
{
    const std::vector<char>& input_type = bundle_native_->GetInputType();
    char output_type = bundle_native_->GetOutputType();
    int32_t input_count = input_type.size();
    int32_t input_width = bundle_native_->GetInputWidth();
    int32_t extend_count = input_count + kMinJniArgCount;

    // Prepare the buffers for input and output manipulation.
    std::unique_ptr<void*[]> arguments(new(std::nothrow) void*[input_width]);
    if (arguments.get() == nullptr)
        CAT(FATAL) << StringPrintf("Allocate buffer to store raw arguments.");

    std::unique_ptr<void*[]> gen_value(new(std::nothrow) void*[extend_count]);
    if (gen_value.get() == nullptr)
        CAT(FATAL) << StringPrintf("Allocate buffer for libffi value array.");

    std::unique_ptr<ffi_type*[]> gen_type(new(std::nothrow) ffi_type*[extend_count]);
    if (gen_type.get() == nullptr)
        CAT(FATAL) << StringPrintf("Allocate buffer for libffi type array.");

    std::string sig_class(kNormObject);
    auto iter = g_map_class_cache->find(sig_class);
    jclass clazz = iter->second->GetClass();
    jobjectArray input_box = env_->NewObjectArray(input_count, clazz, nullptr);
    CHK_EXCP(env_, exit(EXIT_FAILURE));
    gc_auto_.push_back(input_box);

    // Extract the raw input arguments.
    input_marshaller_.Extract(input_type, arguments.get());

    // Prepare the boxed input for the "before-method-execute" instrument callback.
    if (BoxInput(input_box, arguments.get(), input_type) != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Input boxing for \"before-method-execute\" "
                                   "instrument callback.");

    // Invoke the "before-method-execute" instrument callback if necessary.
    jobject bundle_java = bundle_native_->GetBundleObject();
    jmethodID meth_before_exec = bundle_native_->GetBeforeExecuteCallback();
    if (meth_before_exec) {
        env_->CallVoidMethod(bundle_java, meth_before_exec, input_box);
        CHK_EXCP(env_, exit(EXIT_FAILURE));
    }
    // Consume the boxed input which maybe modified by "before-method-execute"
    // instrument callback.
    if (UnboxInput(input_box, arguments.get(), input_type) != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Input unboxing for \"before-method-execute\" "
                                   "instrument callback.");

    // Prepare the generic argument lists for libffi to invoke the original method.
    void* receiver = input_marshaller_.GetReceiver();
    jobject ref_receiver = AddIndirectReference(ref_table_, cookie_, receiver);
    gc_manual_.push_back(ref_receiver);
    jmethodID meth_origin = input_marshaller_.GetMethodID();
    jclass clazz_origin = bundle_native_->GetClass();
    MakeGenericInput(arguments.get(), input_type, &ref_receiver, &clazz_origin,
                     &meth_origin, gen_type.get(), gen_value.get());

    void* result[kWidthQword];
    if (InvokeOrigin(extend_count, meth_origin, output_type, gen_type.get(),
                     gen_value.get(), result) != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Invoke %s.%s%s",
                        bundle_native_->GetClassName().c_str(),
                        bundle_native_->GetMethodName().c_str(),
                        bundle_native_->GetMethodSignature().c_str());

    // Prepare the boxed output for the "after-method-execute" instrument callback.
    jobject output_box;
    if (BoxOutput(&output_box, result, output_type) != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Output boxing for \"after-method-execute\" "
                                   "instrument callback.");

    // Invoke the "after-method-execute" instrument callback if necessary.
    jmethodID meth_after_exec = bundle_native_->GetAfterExecuteCallback();
    if (meth_after_exec) {
        env_->CallVoidMethod(bundle_java, meth_after_exec, output_box);
        CHK_EXCP(env_, exit(EXIT_FAILURE));
    }

    // Consume the boxed output which maybe modified by "after-method-execute"
    // instrument callback.
    if (UnboxOutput(output_box, result, output_type) != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Output unboxing for \"after-method-execute\" "
                                   "instrument callback.");

    // Inject the raw return value for caller consumption.
    output_marshaller_.Inject(output_type, result);

    for (jobject obj : gc_auto_)
        env_->DeleteLocalRef(obj);
    for (jobject obj : gc_manual_)
        RemoveIndirectReference(ref_table_, cookie_, obj);
}

bool MarshallingYard::BoxInput(jobjectArray input_box, void** scan,
                               const std::vector<char>& input_type)
{
    off_t idx = 0;
    for (char type : input_type) {
        jobject obj;
        if (EncapsulateObject(type, false, &scan, &obj) == PROC_FAIL)
            return PROC_FAIL;
        env_->SetObjectArrayElement(input_box, idx++, obj);
        CHK_EXCP_AND_RET_FAIL(env_);
    }
    return PROC_SUCC;
}

bool MarshallingYard::BoxOutput(jobject* p_obj, void** scan, char output_type)
{
    return EncapsulateObject(output_type, true, &scan, p_obj);
}

bool MarshallingYard::UnboxInput(jobjectArray input_box, void** scan,
                                 const std::vector<char>& input_type)
{
    off_t idx = 0;
    for (char type : input_type) {
        jobject obj = env_->GetObjectArrayElement(input_box, idx++);
        CHK_EXCP_AND_RET_FAIL(env_);
        if (DecapsulateObject(type, false, &scan, obj) == PROC_FAIL)
            return PROC_FAIL;
    }
    return PROC_SUCC;
}

bool MarshallingYard::UnboxOutput(jobject obj, void** scan, char output_type)
{
    return DecapsulateObject(output_type, true, &scan, obj);
}

inline bool MarshallingYard::EncapsulateObject(char type, bool is_objref,
                                               void*** p_scan, jobject* p_obj)
{
    if (type == kTypeVoid) {
        *p_obj = nullptr;
        return PROC_SUCC;
    }

    jmethodID meth_ctor;
    jclass clazz;
    if (type != kTypeObject) {
        auto iter = g_map_primitive_wrapper->find(type);
        std::unique_ptr<PrimitiveTypeWrapper>& wrapper = iter->second;
        clazz = wrapper->GetClass();
        meth_ctor = wrapper->GetConstructor();
    }

    void** scan = *p_scan;
    switch (type) {
        case kTypeBoolean: {
            uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
            jboolean real = static_cast<jboolean>(inter);
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            break;
        }
        case kTypeByte: {
            uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
            jbyte real = static_cast<jbyte>(inter);
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            break;
        }
        case kTypeChar: {
            uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
            jchar real = static_cast<jchar>(inter);
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            break;
        }
        case kTypeShort: {
            uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
            jshort real = static_cast<jshort>(inter);
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            break;
        }
        case kTypeInt: {
            uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
            jint real = static_cast<jint>(inter);
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            break;
        }
        case kTypeLong: {
            jlong* deref = reinterpret_cast<jlong*>(scan);
            jlong real = *deref;
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            scan += kWidthQword;
            break;
        }
        case kTypeFloat:
        case kTypeDouble: {
            jdouble* deref = reinterpret_cast<jdouble*>(scan);
            jdouble real = *deref;
            *p_obj = env_->NewObject(clazz, meth_ctor, real);
            CHK_EXCP_AND_RET_FAIL(env_);
            scan += kWidthQword;
            break;
        }
        case kTypeObject: {
            void* obj = *scan++;
            if (is_objref)
                *p_obj = reinterpret_cast<jobject>(obj);
            else {
                if (obj) {
                    *p_obj = AddIndirectReference(ref_table_, cookie_, obj);
                    gc_manual_.push_back(*p_obj);
                } else
                    *p_obj = nullptr;
            }
            break;
        }
    }

    *p_scan = scan;
    return PROC_SUCC;
}

inline bool MarshallingYard::DecapsulateObject(char type, bool must_decode_ref,
                                               void*** p_scan, jobject obj)
{
    if (type == kTypeVoid)
        return PROC_SUCC;

    gc_auto_.push_back(obj);
    jmethodID meth_access;
    jclass clazz;
    if (type != kTypeObject) {
        auto iter = g_map_primitive_wrapper->find(type);
        std::unique_ptr<PrimitiveTypeWrapper>& wrapper = iter->second;
        clazz = wrapper->GetClass();
        meth_access = wrapper->GetAccessor();
    }

    void** scan = *p_scan;
    switch (type) {
        case kTypeBoolean: {
            jboolean value = env_->CallBooleanMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            uintptr_t cast = static_cast<uintptr_t>(value);
            *scan++ = reinterpret_cast<void*>(cast);
            break;
        }
        case kTypeByte: {
            jbyte value = env_->CallByteMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            uintptr_t cast = static_cast<uintptr_t>(value);
            *scan++ = reinterpret_cast<void*>(cast);
            break;
        }
        case kTypeChar: {
            jchar value = env_->CallCharMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            uintptr_t cast = static_cast<uintptr_t>(value);
            *scan++ = reinterpret_cast<void*>(cast);
            break;
        }
        case kTypeShort: {
            jshort value = env_->CallShortMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            uintptr_t cast = static_cast<uintptr_t>(value);
            *scan++ = reinterpret_cast<void*>(cast);
            break;
        }
        case kTypeInt: {
            jint value = env_->CallIntMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            *scan++ = reinterpret_cast<void*>(value);
            break;
        }
        case kTypeLong: {
            jlong value = env_->CallLongMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            jlong* deref = reinterpret_cast<jlong*>(scan);
            *deref = value;
            scan += kWidthQword;
            break;
        }
        case kTypeFloat: {
            jdouble value = env_->CallFloatMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            jdouble* deref = reinterpret_cast<jdouble*>(scan);
            *deref = value;
            scan += kWidthQword;
            break;
        }
        case kTypeDouble: {
            jdouble value = env_->CallDoubleMethod(obj, meth_access);
            CHK_EXCP_AND_RET_FAIL(env_);
            jdouble* deref = reinterpret_cast<jdouble*>(scan);
            *deref = value;
            scan += kWidthQword;
            break;
        }
        case kTypeObject: {
            if (must_decode_ref) {
                if (obj)
                    *scan++ = DecodeJObject(thread_, obj);
                else
                    *scan++ = nullptr;
            }
            else
                *scan++ = obj;
            break;
        }
    }

    *p_scan = scan;
    return PROC_SUCC;
}

void MarshallingYard::MakeGenericInput(void** scan, const std::vector<char>& input_type,
                      jobject* p_ref_receiver, jclass* p_clazz, jmethodID* p_meth,
                      ffi_type** gen_type, void** gen_value)
{
    *gen_type++ = &ffi_type_pointer;
    *gen_type++ = &ffi_type_pointer;
    *gen_type++ = &ffi_type_pointer;

    *gen_value++ = &env_;
    if (bundle_native_->IsStatic()) {
        // For static method call (JNIEnv*, jclass, jmethodID, ...).
        *gen_value++ = p_clazz;
    } else
        // For virtual method call (JNIEnv*, jobject, jmethodID, ...).
        *gen_value++ = p_ref_receiver;
    *gen_value++ = p_meth;

    for (char type : input_type) {
        switch (type) {
            case kTypeBoolean:
                *gen_type++ = &ffi_type_uint8;
                *gen_value++ = scan++;
                break;
            case kTypeByte:
                *gen_type++ = &ffi_type_sint8;
                *gen_value++ = scan++;
                break;
            case kTypeChar:
                *gen_type++ = &ffi_type_uint16;
                *gen_value++ = scan++;
                break;
            case kTypeShort:
                *gen_type++ = &ffi_type_sint16;
                *gen_value++ = scan++;
                break;
            case kTypeInt:
                *gen_type++ = &ffi_type_sint32;
                *gen_value++ = scan++;
                break;
            case kTypeLong:
                *gen_type++ = &ffi_type_sint64;
                *gen_value++ = scan;
                scan += kWidthQword;
                break;
            case kTypeFloat:
            case kTypeDouble:
                *gen_type++ = &ffi_type_double;
                *gen_value++ = scan;
                scan += kWidthQword;
                break;
            case kTypeObject:
                *gen_type++ = &ffi_type_pointer;
                *gen_value++ = scan++;
                break;
        }
    }
}

bool MarshallingYard::InvokeOrigin(int32_t extend_count, jmethodID meth,
        char output_type, ffi_type** gen_type, void** gen_value, void** p_result)
{
    typedef void (*GENFUNC) ();
    #define FFI_CALL(p_value)                                                       \
    do {                                                                            \
        std::lock_guard<std::mutex> guard(mutex);                                   \
        art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry_origin); \
        ffi_call(&cif, func, p_value, gen_value);                                   \
        art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry_hook);   \
    } while (0);

    art::ArtMethod* art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    bool is_static = bundle_native_->IsStatic();
    std::mutex& mutex = bundle_native_->GetMutex();
    uint64_t entry_origin = bundle_native_->GetQuickCodeOriginalEntry();
    uint64_t entry_hook = reinterpret_cast<uint64_t>(ArtQuickInstrumentTrampoline);
    ffi_cif cif;
    GENFUNC func;

    switch (output_type) {
        case kTypeVoid: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_void, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for no return.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticVoidMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallVoidMethod);
            FFI_CALL(nullptr);
            break;
        }
        case kTypeBoolean: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_uint8, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for boolean returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticBooleanMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallBooleanMethod);
            jboolean value;
            FFI_CALL(&value);
            *reinterpret_cast<jboolean*>(p_result) = value;
            break;
        }
        case kTypeByte: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_sint8, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for byte returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticByteMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallByteMethod);
            jbyte value;
            FFI_CALL(&value);
            *reinterpret_cast<jbyte*>(p_result) = value;
            break;
        }
        case kTypeChar: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_uint16, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for char returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticCharMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallCharMethod);
            jchar value;
            FFI_CALL(&value);
            *reinterpret_cast<jchar*>(p_result) = value;
            break;
        }
        case kTypeShort: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_sint16, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for short returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticShortMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallShortMethod);
            jshort value;
            FFI_CALL(&value);
            *reinterpret_cast<jshort*>(p_result) = value;
            break;
        }
        case kTypeInt: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_sint32, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for int returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticIntMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallIntMethod);
            jint value;
            FFI_CALL(&value);
            *reinterpret_cast<jint*>(p_result) = value;
            break;
        }
        case kTypeLong: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_sint64, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for long returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticLongMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallLongMethod);
            jlong value;
            FFI_CALL(&value);
            *reinterpret_cast<jlong*>(p_result) = value;
            break;
        }
        case kTypeFloat: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_float, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for float returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticFloatMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallFloatMethod);
            jfloat value;
            FFI_CALL(&value);
            *reinterpret_cast<jfloat*>(p_result) = value;
            break;
        }
        case kTypeDouble: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_double, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for double returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticDoubleMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallDoubleMethod);
            jdouble value;
            FFI_CALL(&value);
            *reinterpret_cast<jdouble*>(p_result) = value;
            break;
        }
        case kTypeObject: {
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, extend_count,
                             &ffi_type_pointer, gen_type) != FFI_OK)
                CAT(ERROR) << StringPrintf("FFI call for object returned.");
            if (is_static)
                func = reinterpret_cast<GENFUNC>(env_->functions->CallStaticObjectMethod);
            else
                func = reinterpret_cast<GENFUNC>(env_->functions->CallObjectMethod);
            jobject value;
            FFI_CALL(&value);
            *reinterpret_cast<jobject*>(p_result) = value;
            break;
        }
    }

    return PROC_SUCC;
}
