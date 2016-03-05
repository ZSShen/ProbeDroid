#ifndef _JNI_EXCEPT_H_
#define _JNI_EXCEPT_H_


#include <jni.h>


enum {
    JNI_SUCCESS = 0,
    JNI_FAILURE = 1
};

void LogJNIException(JNIEnv*, jthrowable);

static const char* kRecursiveExcept = "Exception occurs during exception message processing.";

#define CHECK_AND_LOG_EXCEPTION(jvm, env)                                      \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogJNIException(env, except);                                  \
                jvm->DetachCurrentThread();                                    \
                return JNI_FAILURE;                                            \
            }                                                                  \
        } while (0);                                                           \

#endif