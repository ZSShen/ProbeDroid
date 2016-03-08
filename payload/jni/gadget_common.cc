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


void InstrumentGadgetComposer::compose()
{
    if (LinkWithAnalysisAPK() != PROC_SUCC)
        return;

    // Initialize the global map for method information maintenance.
    typedef std::unordered_map<void*, std::unique_ptr<MethodBundleNative>>
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
    snprintf(sig, kBlahSizeMid, "(%s)V", kSigString);
    jmethodID meth = env_->GetMethodID(g_class_analysis_main,
                                       kFuncPrepareNativeBridge, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    // Convert the library pathname to UTF format.
    jstring path_module = env_->NewStringUTF(g_lib_path);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    // Invoke it to let the analysis APK link with this native library.
    env_->CallVoidMethod(g_obj_analysis_main, meth, path_module);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    env_->DeleteLocalRef(path_module);
    return PROC_SUCC;
}

bool InstrumentGadgetComposer::RegisterInstrumentGadget()
{
    // Resolve the overrode "void Instrument.onApplicationStart()".
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()V");
    jmethodID meth = env_->GetMethodID(g_class_analysis_main,
                                       kFuncOnApplicationStart, sig);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    // Invoke it to let the analysis APK deploy the instrument gadget.
    env_->CallVoidMethod(g_obj_analysis_main, meth);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    return PROC_SUCC;
}