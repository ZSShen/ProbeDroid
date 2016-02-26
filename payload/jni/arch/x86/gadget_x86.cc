#include "gadget.h"
#include "indirect_reference_table.h"
#include "jni_internal.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"


void* CompileHookGadget(void *obj, void *meth, void *arg_first,
                        void *arg_second, void *stk_ptr)
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    // Cast JNIEnv* to JNIEnvExt* which is the real data type of JNI handle.
    JNIEnvExt* env_ext = reinterpret_cast<JNIEnvExt*>(env);

    // Resolve some important members of JNIEnvExt for resource management.
    uint32_t cookie = env_ext->local_ref_cookie_;
    IndirectReferenceTable* ref_table = reinterpret_cast<IndirectReferenceTable*>
                                        (&(env_ext->local_refs_table_));
    void* thread = env_ext->thread_;

    // Insert the receiver and the first argument into the local indirect
    // reference table, and the reference key is returned.
    jobject ref_obj = AddIndirectReference(ref_table, cookie, obj);
    jobject ref_arg_first = AddIndirectReference(ref_table, cookie, arg_first);

    // The main logic of the hooking gadget compiler.
    //          *** will be updated later ***

    // Restore the entry point to the quick compiled code of "loadClass()".
    art::ArtMethod* art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    uint64_t entry = reinterpret_cast<uint64_t>(g_load_class_quick_compiled);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry);

    // Let "loadClass()" finish its original task. The "android.app.Application"
    // will be returned.
    jmethodID meth_id = reinterpret_cast<jmethodID>(meth);
    jobject ref_clazz = env->CallObjectMethod(ref_obj, meth_id, ref_arg_first);

    // Use the reference key to resolve the actual object.
    void* clazz = DecodeJObject(thread, ref_clazz);

    // Remove the relevant entries of the local indirect reference table.
    RemoveIndirectReference(ref_table, cookie, ref_obj);
    RemoveIndirectReference(ref_table, cookie, ref_arg_first);
    RemoveIndirectReference(ref_table, cookie, ref_clazz);

    LOGD("Success");
    return clazz;
}