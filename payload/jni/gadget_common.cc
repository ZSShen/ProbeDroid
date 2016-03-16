#include <new>

#include "gadget.h"
#include "jni_except-inl.h"


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

// The global map to maintain the information about all the instrumented methods
// of the target app.
PtrBundleMap g_map_method_bundle(nullptr);

// The global map to cache the access information about all the wrappers of
// primitive Java types.
PtrTypeMap g_map_type_wrapper(nullptr);


void InstrumentGadgetComposer::Compose()
{
    if (LinkWithAnalysisAPK() != PROC_SUCC)
        return;

    // Initialize the global map for method information maintenance.
    typedef std::unordered_map<jmethodID, std::unique_ptr<MethodBundleNative>>
            BundleMap;
    BundleMap* bundle_map = new(std::nothrow)BundleMap();
    if (!bundle_map)
        return;
    g_map_method_bundle.reset(bundle_map);

    // Invoke the overrode "void Instrument.onApplicationStart()" to let the
    // analysis APK register the methods which it intends to instrument.
    if (RegisterInstrumentGadget() != PROC_SUCC)
        return;

    return;
}

bool InstrumentGadgetComposer::LinkWithAnalysisAPK()
{
    // Resolve "void Instrument.prepareNativeBridge(String)".
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "(%s)%c", kSigString, kSigVoid);
    jmethodID meth = env_->GetMethodID(g_class_analysis_main,
                                       kFuncPrepareNativeBridge, sig);
    CHK_EXCP(env_);

    // Convert the library pathname to UTF format.
    jstring path_module = env_->NewStringUTF(g_lib_path);
    CHK_EXCP(env_);

    // Invoke it to let the analysis APK link with this native library.
    env_->CallVoidMethod(g_obj_analysis_main, meth, path_module);
    CHK_EXCP(env_);

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
    CHK_EXCP(env_);

    // Invoke it to let the analysis APK deploy the instrument gadget.
    env_->CallVoidMethod(g_obj_analysis_main, meth);
    CHK_EXCP(env_);

    return PROC_SUCC;
}

bool PrimitiveTypeWrapper::LoadWrappers(JNIEnv* env)
{
    // Initialize the global map for primitive type wrapper maintenance.
    typedef std::unordered_map<char, std::unique_ptr<PrimitiveTypeWrapper>>
            TypeMap;
    TypeMap* type_map = new(std::nothrow)TypeMap();
    if (!type_map)
        return PROC_FAIL;
    g_map_type_wrapper.reset(type_map);

    // Load Boolean wrapper.
    char sig[kBlahSizeMid];
    jclass clazz = env->FindClass(kSigBooleanObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigBoolean, kSigVoid);
    jmethodID meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigBoolean);
    jmethodID meth_access = env->GetMethodID(clazz, kFuncBooleanValue, sig);
    CHK_EXCP(env);

    PrimitiveTypeWrapper* wrapper = new(std::nothrow) PrimitiveTypeWrapper(
                                                clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeBoolean,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Byte Wrapper.
    clazz = env->FindClass(kSigByteObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigByte, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigByte);
    meth_access = env->GetMethodID(clazz, kFuncByteValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeByte,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Character Wrapper.
    clazz = env->FindClass(kSigCharObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigChar, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigChar);
    meth_access = env->GetMethodID(clazz, kFuncCharValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeChar,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Short Wrapper.
    clazz = env->FindClass(kSigShortObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigShort, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigShort);
    meth_access = env->GetMethodID(clazz, kFuncShortValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeShort,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Integer Wrapper.
    clazz = env->FindClass(kSigIntObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigInt, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigInt);
    meth_access = env->GetMethodID(clazz, kFuncIntValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeInt,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Float Wrapper.
    clazz = env->FindClass(kSigFloatObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigFloat, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigFloat);
    meth_access = env->GetMethodID(clazz, kFuncFloatValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeFloat,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Long Wrapper.
    clazz = env->FindClass(kSigLongObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigLong, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigLong);
    meth_access = env->GetMethodID(clazz, kFuncLongValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeLong,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Double Wrapper.
    clazz = env->FindClass(kSigDoubleObject);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigDouble, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP(env);
    snprintf(sig, kBlahSizeMid, "()%c", kSigDouble);
    meth_access = env->GetMethodID(clazz, kFuncDoubleValue, sig);
    CHK_EXCP(env);

    wrapper = new(std::nothrow) PrimitiveTypeWrapper(clazz, meth_ctor, meth_access);
    // TODO: Out of memory check and message logging.
    if (!wrapper)
        return PROC_FAIL;
    g_map_type_wrapper->insert(std::make_pair(kTypeDouble,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));
}