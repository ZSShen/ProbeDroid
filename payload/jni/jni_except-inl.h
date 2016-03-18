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