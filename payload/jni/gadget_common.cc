#include "gadget.h"


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

/*
void InstrumentGadgetComposer::compose()
{
    // Invoke "org.probedroid.Instrument.prepareNativeBridge()"" to force the
    // analysis APK link with this native library.
    jstring name_clazz = env_->NewStringUTF("org.zsshen.helloworld.HelloWorld");
    jobject clazz = env_->CallObjectMethod(ref_class_loader_, id_load_class_,
                                           name_clazz);

    jmethodID meth = env->FindMethod(clazz, kFuncPrepareNativeBridge,
                                     kSigPrepareNativeBridge);

    env_->DeleteLocalRef(name_inst);
}
*/