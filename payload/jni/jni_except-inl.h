#include <cstdio>

#include "jni_except.h"
#include "logcat.h"
#include "signature.h"
#include "globals.h"


// TODO: A more fine-grained approach is needed instead of this simple logd spew.
inline void LogException(JNIEnv *env, jthrowable except, const char* name_file,
                         const int line_num)
{
    jclass clazz = env->FindClass(kNormObject);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jboolean is_copy = JNI_FALSE;
    const char* cstr = env->GetStringUTFChars(str, &is_copy);

    LOGD("%s:%d\n%s\n", name_file, line_num, cstr);
    return;
}

inline void ThrowException(JNIEnv *env, const char *type_except, jthrowable except,
                           const char* name_file, const int line_num)
{
    jclass clazz = env->FindClass(kNormObject);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jboolean is_copy = JNI_FALSE;
    const char* cstr = env->GetStringUTFChars(str, &is_copy);

    LOGD("%s:%d\n%s\n", name_file, line_num, cstr);

    clazz = env->FindClass(type_except);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    env->ThrowNew(clazz, cstr);

    return;
}