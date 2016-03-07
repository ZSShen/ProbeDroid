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


void InstrumentGadgetComposer::compose()
{
    if (linkWithAnalysisAPK() != PROC_SUCC)
        return;

    return;
}

bool InstrumentGadgetComposer::linkWithAnalysisAPK()
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

    // Invoke it to force the analysis module link with this native library.
    env_->CallVoidMethod(g_obj_analysis_main, meth, path_module);
    CHECK_AND_LOG_EXCEPTION(g_jvm, env_);

    env_->DeleteLocalRef(path_module);
    return PROC_SUCC;
}