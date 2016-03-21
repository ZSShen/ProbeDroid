#include <memory>
#include <cstdlib>

#include "gadget_x86.h"
#include "gadget.h"
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

void ArtQuickInstrument(void** ret_format, void** ret_value, void* ecx, void* eax,
                        void* edx, void* ebx, void** stack)
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);

    // Use the method id as the key to retrieve the native method bundle.
    jmethodID meth_id = reinterpret_cast<jmethodID>(eax);
    auto iter = g_map_method_bundle->find(meth_id);
    std::unique_ptr<MethodBundleNative>& bundle_native = iter->second;

    // Create the gadgets to extract input arguments and to inject output value
    // with machine specific calling convention.
    InputMarshaller input_marshaller(ecx, eax, ebx, edx, stack);
    OutputMarshaller output_marshaller(ret_format, ret_value);

    // The main process to marshall instrument callbacks.
    MarshallingYard yard(env, bundle_native.get(), input_marshaller, output_marshaller);
    yard.Launch();
}


void InputMarshaller::Extract(int32_t input_width, void** arguments)
{
    if (input_width == 0)
        return;

    if (input_width >= 1)
        *arguments++ = edx_;
    if (input_width >= 2)
        *arguments++ = ebx_;

    int32_t rest = input_width - 2;
    while (rest > 0) {
        *arguments++ = *stack_++;
        --rest;
    }
}

void OutputMarshaller::Inject(char output_type, void** value)
{
    switch (output_type) {
        case kTypeVoid:
            *reinterpret_cast<uint32_t*>(ret_format_) = kNoData;
            *ret_value_ = *value;
            break;
        case kTypeBoolean:
        case kTypeByte:
        case kTypeChar:
        case kTypeShort:
        case kTypeInt:
        case kTypeObject:
            *reinterpret_cast<uint32_t*>(ret_format_) = kDwordInt;
            *ret_value_ = *value;
            break;
        case kTypeFloat:
            *reinterpret_cast<uint32_t*>(ret_format_) = kDwordFloat;
            *ret_value_ = *value;
            break;
        case kTypeLong:
            *reinterpret_cast<uint32_t*>(ret_format_) = kQwordLong;
            *ret_value_++ = *value++;
            *ret_value_ = *value;
            break;
        case kTypeDouble:
            *reinterpret_cast<uint32_t*>(ret_format_) = kQwordDouble;
            *ret_value_++ = *value++;
            *ret_value_ = *value;
            break;
    }
}