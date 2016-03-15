#include <memory>
#include <mutex>

#include "gadget.h"
#include "gadget_x86.h"
#include "indirect_reference_table.h"
#include "jni_internal.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "ffi.h"


void* ComposeInstrumentGadget(void *obj, void *meth, void *arg_first,
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

    // Restore the entry point to the quick compiled code of "loadClass()".
    art::ArtMethod* art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    uint64_t entry = reinterpret_cast<uint64_t>(g_load_class_quick_compiled);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry);

    // Enter the instrument gadget composer.
    jmethodID meth_id = reinterpret_cast<jmethodID>(meth);
    InstrumentGadgetComposer composer(env, ref_obj, meth_id);
    composer.Compose();

    // Let "loadClass()" finish its original task. The "android.app.Application"
    // will be returned.
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

void ArtQuickInstrument(void **ret_type, void **ret_val, void *obj, void *meth,
                        void *reg_first, void *reg_second, void **stk_ptr)
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

    // Use method pointer as the key to retrieve the instrument gadgets.
    jmethodID meth_id = reinterpret_cast<jmethodID>(meth);
    auto iter = g_map_method_bundle->find(meth_id);
    std::unique_ptr<MethodBundleNative>& bundle = iter->second;

    // Ensure that only one thread can process a specific instrumented method simultaneously.
    std::mutex& mutex = bundle->GetMutex();
    {
        std::lock_guard<std::mutex> guard(mutex);
        //void (*ptr)() = reinterpret_cast<void(*)()>(env->functions->CallObjectMethod);
    }
}

