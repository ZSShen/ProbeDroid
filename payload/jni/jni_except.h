#ifndef _JNI_EXCEPT_H_
#define _JNI_EXCEPT_H_


#include <jni.h>


void LogException(JNIEnv*, jthrowable, const char*, const int);
void ThrowException(JNIEnv*, const char*, jthrowable, const char*, const int);

static const char* kRecursiveExcept = "Exception occurs during exception message processing.";


#define CHK_EXCP(jvm, env)                                                     \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogException(env, except, __FILE__, __LINE__);                 \
                return PROC_FAIL;                                              \
            }                                                                  \
        } while (0);

#define CHK_EXCP_AND_DETACH(jvm, env)                                          \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                LogException(env, except, __FILE__, __LINE__);                 \
                jvm->DetachCurrentThread();                                    \
                return PROC_FAIL;                                              \
            }                                                                  \
        } while (0);                                                           \

#define CHK_EXCP_AND_RETHROW(jvm, env, type)                                   \
        do {                                                                   \
            jthrowable except;                                                 \
            if ((except = env->ExceptionOccurred())) {                         \
                env->ExceptionClear();                                         \
                ThrowException(env, type, except, __FILE__, __LINE__);         \
                return;                                                        \
            }                                                                  \
        } while (0);

#endif