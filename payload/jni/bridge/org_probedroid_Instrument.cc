#include <cstdio>
#include <cstring>
#include <vector>
#include <new>

#include "org_probedroid_instrument.h"
#include "globals.h"
#include "signature.h"
#include "gadget.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "jni_except-inl.h"


JNIEXPORT void JNICALL Java_org_probedroid_Instrument_instrumentMethodNative
  (JNIEnv *env, jobject thiz, jboolean is_static, jstring name_class,
   jstring name_method, jstring signature_method, jobject bundle)
{
    jboolean is_copy = JNI_FALSE;
    if (env->IsSameObject(name_class, nullptr) == JNI_TRUE ||
        env->IsSameObject(name_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(signature_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(bundle, nullptr) == JNI_TRUE)
        CHK_EXCP_AND_RETHROW(g_jvm, env, kSigIllegalArgument);

    const char* cstr_class_name = env->GetStringUTFChars(name_class, &is_copy);
    const char* cstr_method_name = env->GetStringUTFChars(name_method, &is_copy);
    const char* cstr_method_sig = env->GetStringUTFChars(signature_method, &is_copy);

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
    jclass clazz = env->FindClass(sig);
    CHK_EXCP_AND_RETHROW(g_jvm, env, kSigClassNotFound);
    jmethodID meth_tge;
    if (is_static)
        meth_tge = env->GetStaticMethodID(clazz, cstr_method_name, cstr_method_sig);
    else
        meth_tge = env->GetMethodID(clazz, cstr_method_name, cstr_method_sig);
    CHK_EXCP_AND_RETHROW(g_jvm, env, kSigNoSuchMethod);

    // Get the entry point to the quick compiled code of that method.
    art::ArtMethod *art_meth = reinterpret_cast<art::ArtMethod*>(meth_tge);
    uint64_t entry_origin = art::ArtMethod::GetEntryPointFromQuickCompiledCode(art_meth);

    // Resolve the instrument bundle passed by analysis APK to check:
    // (1) If there is any necessary callback before target method execution.
    // (2) If there is any necessary callback after target method execution.
    jclass clazz_bundle = env->GetObjectClass(bundle);

    snprintf(sig, kBlahSizeMid, "%c", kSigBoolean);
    jfieldID fld_before = env->GetFieldID(clazz_bundle, kFieldInterceptBefore, sig);
    CHK_EXCP_AND_RETHROW(g_jvm, env, kSigIllegalArgument);
    jfieldID fld_after = env->GetFieldID(clazz_bundle, kFieldInterceptAfter, sig);
    CHK_EXCP_AND_RETHROW(g_jvm, env, kSigIllegalArgument);

    jboolean before = env->GetBooleanField(bundle, fld_before);
    jboolean after = env->GetBooleanField(bundle, fld_after);

    // Get the entry points to the quick compiled code of the instrument callbacks.
    jmethodID meth_before, meth_after;
    if (before) {
        snprintf(sig, kBlahSizeMid, "(%c%s)%c", kSigArray, kSigObjectLong, kSigVoid);
        meth_before = env->GetMethodID(clazz_bundle, kFuncBeforeMethodExecute, sig);
        CHK_EXCP_AND_RETHROW(g_jvm, env, kSigIllegalArgument);
    }
    if (after) {
        snprintf(sig, kBlahSizeMid, "(%s)%c", kSigObjectLong, kSigVoid);
        meth_after = env->GetMethodID(clazz_bundle, kFuncAfterMethodExecute, sig);
        CHK_EXCP_AND_RETHROW(g_jvm, env, kSigIllegalArgument);
    }

    // Parse the method signature to acquire the relevant data types.
    MethodSignatureParser parser(cstr_method_sig);
    parser.Parse();
    std::vector<char>& type_inputs = parser.GetInputType();
    char type_output = parser.GetOutputType();

    MethodBundleNative* bundle_native = new(std::nothrow) MethodBundleNative(
        is_static, cstr_class_name, cstr_method_name, cstr_method_sig,
        type_inputs, type_output, entry_origin, meth_before, meth_after);
    // TODO: Out of memory check and message logging.
    if (!bundle_native)
        return;

    g_map_method_bundle->insert(std::make_pair(meth_tge,
                           std::unique_ptr<MethodBundleNative>(bundle_native)));
    return;
}