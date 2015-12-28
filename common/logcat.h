#ifndef _LOGCAT_H_
#define _LOGCAT_H_


#include "android/log.h"


#define LOG_TAG "PAYLOAD"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)

#endif