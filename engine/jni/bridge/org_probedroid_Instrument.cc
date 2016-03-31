#include <cstdio>
#include <cstring>
#include <vector>
#include <new>

#include "org_probedroid_instrument.h"
#include "globals.h"
#include "gadget.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "jni_except-inl.h"
#include "stringprintf.h"


JNIEXPORT void JNICALL Java_org_probedroid_Instrument_instrumentMethodNative
  (JNIEnv *env, jobject thiz, jboolean is_static, jstring name_class,
   jstring name_method, jstring signature_method, jobject bundle)
{
    jboolean is_copy = JNI_FALSE;
    if (env->IsSameObject(name_class, nullptr) == JNI_TRUE ||
        env->IsSameObject(name_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(signature_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(bundle, nullptr) == JNI_TRUE)
        CHK_EXCP_AND_RET(env, RETHROW(kNormIllegalArgument));

    const char* cstr_class_name = env->GetStringUTFChars(name_class, &is_copy);
    const char* cstr_method_name = env->GetStringUTFChars(name_method, &is_copy);
    const char* cstr_method_sig = env->GetStringUTFChars(signature_method, &is_copy);
    if (!cstr_class_name || !cstr_method_name || !cstr_method_sig) {
        jthrowable except;
        RETHROW(kNormIllegalArgument);
    }

    // Load the hosting class of the to be instrumented method.
    jobject ref_class = env->CallObjectMethod(g_ref_class_loader,
                                              g_meth_load_class, name_class);
    CHK_EXCP_AND_RET(env, RETHROW(kNormClassNotFound));
    jclass clazz = reinterpret_cast<jclass>(ref_class);

    // Normalize the class name.
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "%s", cstr_class_name);
    char* ofst = sig;
    while (*ofst) {
        if (*ofst == kDeliDot)
            *ofst = kDeliSlash;
        ++ofst;
    }

    // Resolve the to be instrumented method.
    //jclass clazz = env->FindClass(sig);
    //CHK_EXCP_AND_RET(env, RETHROW(kNormClassNotFound));
    jmethodID meth_tge;
    if (is_static)
        meth_tge = env->GetStaticMethodID(clazz, cstr_method_name, cstr_method_sig);
    else
        meth_tge = env->GetMethodID(clazz, cstr_method_name, cstr_method_sig);
    CHK_EXCP_AND_RET(env, RETHROW(kNormNoSuchMethod));

    // Get the entry point to the quick compiled code of that method.
    art::ArtMethod *art_meth = reinterpret_cast<art::ArtMethod*>(meth_tge);
    uint64_t entry_origin = art::ArtMethod::GetEntryPointFromQuickCompiledCode(art_meth);

    // Resolve the instrument bundle passed by analysis APK to check:
    // (1) If there is any necessary callback before target method execution.
    // (2) If there is any necessary callback after target method execution.
    jclass clazz_bundle = env->GetObjectClass(bundle);

    snprintf(sig, kBlahSizeMid, "%c", kSigBoolean);
    jfieldID fld_before = env->GetFieldID(clazz_bundle, kFieldInterceptBefore, sig);
    CHK_EXCP_AND_RET(env, RETHROW(kNormIllegalArgument));
    jfieldID fld_after = env->GetFieldID(clazz_bundle, kFieldInterceptAfter, sig);
    CHK_EXCP_AND_RET(env, RETHROW(kNormIllegalArgument));

    jboolean before = env->GetBooleanField(bundle, fld_before);
    jboolean after = env->GetBooleanField(bundle, fld_after);

    // Get the entry points to the quick compiled code of the instrument callbacks.
    jmethodID meth_before = nullptr, meth_after = nullptr;
    if (before) {
        snprintf(sig, kBlahSizeMid, "(%c%s)%c", kSigArray, kSigObjectObject, kSigVoid);
        meth_before = env->GetMethodID(clazz_bundle, kFuncBeforeMethodExecute, sig);
        CHK_EXCP_AND_RET(env, RETHROW(kNormIllegalArgument));
    }
    if (after) {
        snprintf(sig, kBlahSizeMid, "(%s)%c", kSigObjectObject, kSigVoid);
        meth_after = env->GetMethodID(clazz_bundle, kFuncAfterMethodExecute, sig);
        CHK_EXCP_AND_RET(env, RETHROW(kNormIllegalArgument));
    }

    // Parse the method signature to acquire the relevant data types.
    MethodSignatureParser parser(cstr_method_sig);
    parser.Parse();
    const std::vector<char>& type_inputs = parser.GetInputType();
    char type_output = parser.GetOutputType();

    jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref)
        CAT(FATAL) << StringPrintf("Allocate global reference for class %s",
                                   cstr_class_name);
    jclass g_clazz = reinterpret_cast<jclass>(g_ref);

    jobject g_bundle = env->NewGlobalRef(bundle);
    if (!g_bundle)
        CAT(FATAL) << StringPrintf("Allocate global reference for a MethodBundle.");

    MethodBundleNative* bundle_native = new(std::nothrow) MethodBundleNative(
        is_static, cstr_class_name, cstr_method_name, cstr_method_sig, type_inputs,
        type_output, g_clazz, entry_origin, g_bundle, meth_before, meth_after);
    if (!bundle_native)
        CAT(FATAL) << StringPrintf("Allocate MethodBundleNative for %s.%s%s",
                        cstr_class_name, cstr_method_name, cstr_method_sig);

    g_map_method_bundle->insert(std::make_pair(meth_tge,
                           std::unique_ptr<MethodBundleNative>(bundle_native)));

    // Replace the entry of quick compiled code by our instrument trampoline.
    uint64_t entry_instrument = reinterpret_cast<uint64_t>(ArtQuickInstrumentTrampoline);
    art::ArtMethod::SetEntryPointFromQuickCompiledCode(art_meth, entry_instrument);

    return;
}