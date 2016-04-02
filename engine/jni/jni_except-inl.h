/**
 *   The MIT License (MIT)
 *   Copyright (c) <2016> <ZongXian Shen, andy.zsshen@gmail.com>
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

#include "jni_except.h"
#include "logcat.h"
#include "signature.h"
#include "globals.h"
#include "stringprintf.h"


inline void LogException(JNIEnv *env, jthrowable except)
{
    std::string sig_class(kNormObject);
    auto iter_class = g_map_class_cache->find(kNormObject);
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "%s()%s", kFuncToString, kSigString);
    std::string sig_method(sig);
    jmethodID meth = iter_class->second->GetCachedMethod(sig_method);

    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        CAT(ERROR) << StringPrintf("%s", kRecursiveExcept);
        return;
    }
    jboolean is_copy = JNI_FALSE;
    const char* msg = env->GetStringUTFChars(str, &is_copy);

    CAT(ERROR) << StringPrintf("%s", msg);
    return;
}

inline void ThrowException(JNIEnv *env, jthrowable except, const char *type_except)
{
    std::string sig_class(kNormObject);
    auto iter_class = g_map_class_cache->find(kNormObject);
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "%s()%s", kFuncToString, kSigString);
    std::string sig_method(sig);
    jmethodID meth = iter_class->second->GetCachedMethod(sig_method);

    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        CAT(ERROR) << StringPrintf("%s", kRecursiveExcept);
        return;
    }
    jboolean is_copy = JNI_FALSE;
    const char* msg = env->GetStringUTFChars(str, &is_copy);

    std::string sig_class_except(type_except);
    iter_class = g_map_class_cache->find(sig_class_except);
    jclass clazz = iter_class->second->GetClass();
    env->ThrowNew(clazz, msg);
    return;
}