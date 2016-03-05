#include "jni_except.h"
#include "logcat.h"


// TODO: A more fine-grained approach is needed instead of this simple logd spew.
inline void LogJNIException(JNIEnv *env, jthrowable except)
{
    jclass clazz = env->FindClass(kNormObject);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    char sig[kBlahSizeMid];
    snprintf(sig, kBlahSizeMid, "()%s", kSigString);
    jmethodID meth = env->GetMethodID(clazz, kFuncToString, sig);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jstring str = reinterpret_cast<jstring>(env->CallObjectMethod(except, meth));
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    jboolean is_copy = false;
    const char* cstr = env->GetStringUTFChars(str, &is_copy);
    if (env->ExceptionCheck()) {
        LOGD("%s\n", kRecursiveExcept);
        return;
    }
    LOGD("%s\n", cstr);
}