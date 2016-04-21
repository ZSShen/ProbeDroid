/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

#include <cstdio>
#include <cstring>
#include <vector>
#include <new>
#include <setjmp.h>

#include "org_probedroid_instrument.h"
#include "globals.h"
#include "gadget.h"
#include "mirror/art_method-inl.h"
#include "logcat.h"
#include "jni_except-inl.h"
#include "stringprintf.h"


JNIEXPORT jint JNICALL Java_org_probedroid_Instrument_instrumentMethodNative
  (JNIEnv *env, jobject thiz, jboolean is_static, jstring name_class,
   jstring name_method, jstring signature_method, jobject bundle)
{
    // Since our gadgets are dynamically injected, there is no corresponding
    // ArtMethod information. If the successive JNI calls try to throw exception
    // or to generate exception object, Android Runtime will try to build the
    // stack trace which will trigger the unchecked NULL ArtMethod error. This is
    // natural, because Android Runtime does not expect such "unrecorded methods".
    // To solve the issue, we must silence the stack trace.
    CloseRuntimeStackTrace();

    if (env->IsSameObject(name_class, nullptr) == JNI_TRUE ||
        env->IsSameObject(name_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(signature_method, nullptr) == JNI_TRUE ||
        env->IsSameObject(bundle, nullptr) == JNI_TRUE)
        return org_probedroid_Instrument_ERR_EMPTY_STRING;

    if (env->GetStringLength(name_class) == 0 ||
        env->GetStringLength(name_method) == 0 ||
        env->GetStringLength(signature_method) == 0)
        return org_probedroid_Instrument_ERR_EMPTY_STRING;

    jboolean is_copy = JNI_FALSE;
    const char* cstr_class_name = env->GetStringUTFChars(name_class, &is_copy);
    const char* cstr_method_name = env->GetStringUTFChars(name_method, &is_copy);
    const char* cstr_method_sig = env->GetStringUTFChars(signature_method, &is_copy);

    // Remember that we are in ClassLoader.loadClass() called by ActivityThread
    // to load the android.app.Application class. And we just slickly use that
    // class loader to load the to be instrumented code. If the class loader
    // cannot find the class, it expects to return to the ClassNotFound catch
    // block in the Java site. During the backward stack trace, it will enter
    // this gadget. Again, NULL ArtMethod error! To solve the issue, we patch
    // the original exception delivery function and let the control flow directly
    // dive to here without Runtime stack trace. Then we skip this level and
    // re-throw the exception to instrumentation package.
    void* except_original;
    GetFuncDeliverException(&except_original);
    void* except_hooked = reinterpret_cast<void*>(ArtQuickDeliverExceptionTrampoline);
    SetFuncDeliverException(except_hooked);

    // Try to the hosting class of the to be instrumented method.
    if (setjmp(g_save_ptr) != 0) {
        SetFuncDeliverException(except_original);
        return org_probedroid_Instrument_ERR_CLASS_NOT_FOUND;
    }
    jobject ref_class = env->CallObjectMethod(g_ref_class_loader,
                                              g_meth_load_class, name_class);
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
    jmethodID meth_tge;
    if (is_static)
        meth_tge = env->GetStaticMethodID(clazz, cstr_method_name, cstr_method_sig);
    else
        meth_tge = env->GetMethodID(clazz, cstr_method_name, cstr_method_sig);
    if (!meth_tge) {
        env->ExceptionClear();
        SetFuncDeliverException(except_original);
        return org_probedroid_Instrument_ERR_NO_SUCH_METHOD;
    }

    // Restore the exception delivery function.
    SetFuncDeliverException(except_original);

    // Get the entry point to the quick compiled code of that method.
    art::ArtMethod *art_meth = reinterpret_cast<art::ArtMethod*>(meth_tge);
    uint64_t entry_origin = art::ArtMethod::GetEntryPointFromQuickCompiledCode(art_meth);

    // Resolve the instrument bundle passed by analysis APK to check:
    // (1) If there is any necessary callback before target method execution.
    // (2) If there is any necessary callback after target method execution.
    jclass clazz_bundle = env->GetObjectClass(bundle);

    snprintf(sig, kBlahSizeMid, "%c", kSigBoolean);
    jfieldID fld_before = env->GetFieldID(clazz_bundle, kFieldInterceptBefore, sig);
    if (!fld_before)
        return org_probedroid_Instrument_ERR_ABNORMAL_BUNDLE;
    jfieldID fld_after = env->GetFieldID(clazz_bundle, kFieldInterceptAfter, sig);
    if (!fld_after)
        return org_probedroid_Instrument_ERR_ABNORMAL_BUNDLE;

    jboolean before = env->GetBooleanField(bundle, fld_before);
    jboolean after = env->GetBooleanField(bundle, fld_after);

    // Get the entry points to the quick compiled code of the instrument callbacks.
    jmethodID meth_before = nullptr, meth_after = nullptr;
    if (before) {
        snprintf(sig, kBlahSizeMid, "(%c%s)%c", kSigArray, kSigObjectObject, kSigVoid);
        meth_before = env->GetMethodID(clazz_bundle, kFuncBeforeMethodExecute, sig);
        if (!meth_before)
            return org_probedroid_Instrument_ERR_ABNORMAL_BUNDLE;
    }
    if (after) {
        snprintf(sig, kBlahSizeMid, "(%s)%c", kSigObjectObject, kSigVoid);
        meth_after = env->GetMethodID(clazz_bundle, kFuncAfterMethodExecute, sig);
        if (!meth_after)
            return org_probedroid_Instrument_ERR_ABNORMAL_BUNDLE;
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

    return org_probedroid_Instrument_INSTRUMENT_OK;
}