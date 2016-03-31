#ifndef _JNI_EXCEPT_H_
#define _JNI_EXCEPT_H_


#include <jni.h>


void LogException(JNIEnv*, jthrowable);
void ThrowException(JNIEnv*, const char*);

static const char* kRecursiveExcept = "Exception occurs during exception message processing.";


#define CHK_EXCP(env, ...)                                                     \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogException(env, except);                                     \
                __VA_ARGS__;                                                   \
            }                                                                  \
        } while (0);

#define CHK_EXCP_AND_RET(env, ...)                                             \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogException(env, except);                                     \
                __VA_ARGS__;                                                   \
                return;                                                        \
            }                                                                  \
        } while (0);

#define CHK_EXCP_AND_RET_FAIL(env, ...)                                        \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogException(env, except);                                     \
                __VA_ARGS__;                                                   \
                return PROC_FAIL;                                              \
            }                                                                  \
        } while (0);

#define DETACH(jvm)                                                            \
        do {                                                                   \
            jvm->DetachCurrentThread();                                        \
        } while (0);

#define RETHROW(type)                                                          \
        do {                                                                   \
            ThrowException(env, except, type);                                 \
        } while (0);

#endif