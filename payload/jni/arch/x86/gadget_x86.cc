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

    JNIEnvExt* env_ext = reinterpret_cast<JNIEnvExt*>(env);
    uint32_t cookie = env_ext->local_ref_cookie_;
    IndirectReferenceTable* ref_table = reinterpret_cast<IndirectReferenceTable*>
                                        (&(env_ext->local_refs_table_));
    void* thread = env_ext->thread_;

    jobject ref_obj = AddIndirectReference(ref_table, cookie, obj);
    jobject ref_arg_first = AddIndirectReference(ref_table, cookie, arg_first);

    art::ArtMethod* art_meth = reinterpret_cast<art::ArtMethod*>(meth);
    uint64_t entry = reinterpret_cast<uint64_t>(g_load_class_quick_compiled);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry);

    jmethodID meth_id = reinterpret_cast<jmethodID>(meth);
    jobject ref_res = env->CallObjectMethod(ref_obj, meth_id, ref_arg_first);

    void* res = DecodeJObject(thread, ref_res);
    RemoveIndirectReference(ref_table, cookie, ref_obj);
    RemoveIndirectReference(ref_table, cookie, ref_arg_first);
    RemoveIndirectReference(ref_table, cookie, ref_res);

    LOGD("Success");
    return res;
}