#include <cstdlib>

#include "logcat.h"
#include "android/log.h"


#define LOG_TAG "ProbeDroid"
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)

SpewCat::~SpewCat()
{
    char tag = "VDIWEFF"[severity_];
    std::string msg(buffer_.str());
    LOGD("(%c) [%s:%d] %s", tag, file_, line_, msg.c_str());
    if (severity_ == FATAL) {
    	LOGD("Terminiate instrument process!");
        exit(EXIT_FAILURE);
    }
}