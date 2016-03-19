#include <memory>
#include <mutex>
#include <cstdlib>

#include "gadget.h"
#include "gadget_x86.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "ffi.h"
#include "globals.h"
#include "jni_except-inl.h"


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
    CHK_EXCP(env, exit(EXIT_FAILURE));

    // Use the reference key to resolve the actual object.
    void* clazz = DecodeJObject(thread, ref_clazz);

    // Remove the relevant entries of the local indirect reference table.
    RemoveIndirectReference(ref_table, cookie, ref_obj);
    RemoveIndirectReference(ref_table, cookie, ref_arg_first);
    RemoveIndirectReference(ref_table, cookie, ref_clazz);

    CAT(INFO) << StringPrintf("Gadget deployment success.");
    return clazz;
}

void ArtQuickInstrument(void **ret_type, void **ret_val, void *receiver, void *meth,
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
    std::unique_ptr<MethodBundleNative>& bundle_native = iter->second;

    // Create the input marshaller to box the arguments for instrument callback.
    const std::vector<char>& ref_input_type = bundle_native->GetInputTypes();
    int32_t unboxed_input_width = bundle_native->GetUnboxedInputWidth();
    InputMarshaller input_marshaller(env, ref_input_type.size(), unboxed_input_width,
        ref_input_type, receiver, reg_first, reg_second, stk_ptr + kStackAlignment);

    input_marshaller.Flatten();
    if(input_marshaller.BoxInputs() != PROC_SUCC)
        CAT(FATAL) << StringPrintf("Box the input for instrument callback"
            " before %s.%s%s", bundle_native->GetClassName().c_str(),
           bundle_native->GetMethodName().c_str(),
           bundle_native->GetMethodSignature().c_str());

    // Launch the callback to instrument input arguments.
    jobject bundle_java = bundle_native->GetBundleObject();
    jmethodID meth_before_exec = bundle_native->GetBeforeExecuteCallback();
    jobjectArray boxed_input = input_marshaller.GetBoxedInputs();
    env->CallVoidMethod(bundle_java, meth_before_exec, boxed_input);
    CHK_EXCP(env, exit(EXIT_FAILURE));

    CAT(INFO) << StringPrintf("Test OK");
}

void InputMarshaller::Flatten()
{
    if (unboxed_input_width_ == 0)
        return;

    off_t idx = 0;
    if (unboxed_input_width_ >= 1)
        unboxed_inputs_[idx++] = reg_first_;
    if (unboxed_input_width_ >= 2)
        unboxed_inputs_[idx++] = reg_second_;

    int32_t rest = unboxed_input_width_ - 2;
    while (rest > 0) {
        unboxed_inputs_[idx++] = *stk_ptr_++;
        --rest;
    }
}

bool InputMarshaller::BoxInputs()
{
    // Create an array of "java.lang.Object" which stores the boxed inputs.
    std::string sig_class(kNormObject);
    auto iter = g_map_class_cache->find(sig_class);
    jclass clazz = iter->second->GetClass();
    boxed_inputs_ = env_->NewObjectArray(count_input_, clazz, nullptr);
    CHK_EXCP_AND_RET_FAIL(env_);

    // Scanning pointer for boxing process.
    void** scan = unboxed_inputs_.get();

    // Box each argument by referencing its original data type.
    off_t idx = 0;
    for (char type : ref_input_type_) {
        jmethodID meth_ctor;
        jclass clazz;
        jobject obj;
        if (type != kTypeObject) {
            auto iter = g_map_primitive_wrapper->find(type);
            std::unique_ptr<PrimitiveTypeWrapper>& wrapper = iter->second;
            clazz = wrapper->GetClass();
            meth_ctor = wrapper->GetConstructor();
        }

        switch (type) {
            case kTypeBoolean: {
                uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
                jboolean real = static_cast<jboolean>(inter);
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeByte: {
                uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
                jbyte real = static_cast<jbyte>(inter);
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeChar: {
                uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
                jchar real = static_cast<jchar>(inter);
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeShort: {
                uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
                jshort real = static_cast<jshort>(inter);
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeInt: {
                uintptr_t inter = reinterpret_cast<uintptr_t>(*scan++);
                jint real = static_cast<jint>(inter);
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeFloat: {
                jfloat* deref = reinterpret_cast<jfloat*>(scan++);
                jfloat real = *deref;
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                break;
            }
            case kTypeLong: {
                jlong* deref = reinterpret_cast<jlong*>(scan);
                jlong real = *deref;
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                scan += kWidthQword;
                break;
            }
            case kTypeDouble: {
                jdouble* deref = reinterpret_cast<jdouble*>(scan);
                jdouble real = *deref;
                obj = env_->NewObject(clazz, meth_ctor, real);
                CHK_EXCP_AND_RET_FAIL(env_);
                scan += kWidthQword;
                break;
            }
            case kTypeObject: {
                void* ptr_obj = *scan++;
                obj = AddIndirectReference(ref_table_, cookie_, ptr_obj);
                break;
            }
        }

        env_->SetObjectArrayElement(boxed_inputs_, idx++, obj);
    }

    return PROC_SUCC;
}


