#ifndef _GADGET_H_
#define _GADGET_H_


#include <jni.h>
#include <string>
#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "mirror/art_method-inl.h"


typedef void (*OriginalCall) ();

// The gadget to extract JNI handle from TLS.
extern "C" void GetJniEnv(JNIEnv**) __asm__("GetJniEnv");

class Gadget {};

#endif