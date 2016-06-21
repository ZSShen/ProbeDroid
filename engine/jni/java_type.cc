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

#include "java_type.h"
#include "gadget.h"
#include "jni_except.h"


// The global map to cache the access information about all the wrappers of
// primitive Java types.
PtrPrimitiveMap g_map_primitive_wrapper(nullptr);

// The global map to cache the frequently used method ids.
PtrClassMap g_map_class_cache(nullptr);


PrimitiveTypeWrapper::~PrimitiveTypeWrapper()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(clazz_);
}

bool PrimitiveTypeWrapper::LoadWrappers(JNIEnv* env)
{
    // Load Boolean wrapper.
    char sig[kBlahSizeMid];
    jclass clazz = env->FindClass(kNormBooleanObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigBoolean, kSigVoid);
    jmethodID meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigBoolean);
    jmethodID meth_access = env->GetMethodID(clazz, kFuncBooleanValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Boolean class.");
        return PROC_FAIL;
    }
    jclass g_clazz = reinterpret_cast<jclass>(g_ref);
    PrimitiveTypeWrapper* wrapper = new(std::nothrow) PrimitiveTypeWrapper(
                                        g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Boolean.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeBoolean,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Byte Wrapper.
    clazz = env->FindClass(kNormByteObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigByte, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigByte);
    meth_access = env->GetMethodID(clazz, kFuncByteValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Byte class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Byte.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeByte,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Character Wrapper.
    clazz = env->FindClass(kNormCharObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigChar, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigChar);
    meth_access = env->GetMethodID(clazz, kFuncCharValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Character class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Character.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeChar,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Short Wrapper.
    clazz = env->FindClass(kNormShortObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigShort, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigShort);
    meth_access = env->GetMethodID(clazz, kFuncShortValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Short class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Short.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeShort,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Integer Wrapper.
    clazz = env->FindClass(kNormIntObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigInt, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigInt);
    meth_access = env->GetMethodID(clazz, kFuncIntValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Integer class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Integer.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeInt,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Float Wrapper.
    clazz = env->FindClass(kNormFloatObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigFloat, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigFloat);
    meth_access = env->GetMethodID(clazz, kFuncFloatValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Float class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Float.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeFloat,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Long Wrapper.
    clazz = env->FindClass(kNormLongObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigLong, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigLong);
    meth_access = env->GetMethodID(clazz, kFuncLongValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Long class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Long.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeLong,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    // Load Double Wrapper.
    clazz = env->FindClass(kNormDoubleObject);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "(%c)%c", kSigDouble, kSigVoid);
    meth_ctor = env->GetMethodID(clazz, kFuncConstructor, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    snprintf(sig, kBlahSizeMid, "()%c", kSigDouble);
    meth_access = env->GetMethodID(clazz, kFuncDoubleValue, sig);
    CHK_EXCP_AND_RET_FAIL(env);

    g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
    if (!g_ref) {
        CAT(ERROR) << StringPrintf("Allocate a global reference for Double class.");
        return PROC_FAIL;
    }
    g_clazz = reinterpret_cast<jclass>(g_ref);
    wrapper = new(std::nothrow) PrimitiveTypeWrapper(g_clazz, meth_ctor, meth_access);
    if (!wrapper) {
        CAT(ERROR) << StringPrintf("Allocate PrimitiveTypeWrapper for Double.");
        return PROC_FAIL;
    }
    g_map_primitive_wrapper->insert(std::make_pair(kTypeDouble,
                               std::unique_ptr<PrimitiveTypeWrapper>(wrapper)));

    return PROC_SUCC;
}

ClassCache::~ClassCache()
{
    JNIEnv* env;
    g_jvm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(clazz_);
}

bool ClassCache::LoadClasses(JNIEnv* env)
{
    char sig[kBlahSizeMid];
    // Load "java.lang.Object".
    {
        jclass clazz = env->FindClass(kNormObject);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for Object class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for Object.");
            return PROC_FAIL;
        }

        // Load "String Object.toString()".
        {
            snprintf(sig, kBlahSizeMid, "()%s", kSigString);
            jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
            CHK_EXCP_AND_RET_FAIL(env);

            snprintf(sig, kBlahSizeMid, "%s()%s", kFuncToString, kSigString);
            std::string sig_method(sig);
            class_cache->CacheMethod(sig_method, meth);
        }
        std::string sig_class(kNormObject);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.IllegalArgumentException".
    {
        jclass clazz = env->FindClass(kNormIllegalArgument);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "IllegalArgumentException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for IllegalArgumentException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormIllegalArgument);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.ClassNotFoundException".
    {
        jclass clazz = env->FindClass(kNormClassNotFound);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "ClassNotFoundException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow) ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for ClassNotFoundException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormClassNotFound);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    // Load "java.lang.NoSuchMethodException".
    {
        jclass clazz = env->FindClass(kNormNoSuchMethod);
        CHK_EXCP_AND_RET_FAIL(env);

        jobject g_ref = env->NewGlobalRef(reinterpret_cast<jobject>(clazz));
        if (!g_ref) {
            CAT(ERROR) << StringPrintf("Allocate a global reference for "
                                        "NoSuchMethodException class.");
            return PROC_FAIL;
        }

        jclass g_clazz = reinterpret_cast<jclass>(g_ref);
        ClassCache* class_cache = new(std::nothrow)ClassCache(g_clazz);
        if (!class_cache) {
            CAT(ERROR) << StringPrintf("Allocate ClassCache for NoSuchMethodException.");
            return PROC_FAIL;
        }
        std::string sig_class(kNormNoSuchMethod);
        g_map_class_cache->insert(std::make_pair(sig_class,
                            std::unique_ptr<ClassCache>(class_cache)));
    }

    return PROC_SUCC;
}
