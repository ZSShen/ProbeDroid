#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <util.h>
#include <jni.h>
#include <jni_internal.h>


typedef void (*FUNC_INVOKE_VIRTUAL) (uint32_t, void*);
typedef void* (*FUNC_RESOLVE_STRING) (void*, uint32_t);
typedef void (*FUNC_VIRTUAL_METHOD_CALL) (jmethodID);


// Utility to replace function pointers.
extern void HookInvokeVirtual(FUNC_INVOKE_VIRTUAL*, FUNC_INVOKE_VIRTUAL) \
            __asm__("HookInvokeVirtual");

extern void HookResolveString(FUNC_RESOLVE_STRING*, FUNC_RESOLVE_STRING) \
            __asm__("HookResolveString");

extern void GetJNIENVExt(JNIEnv**) __asm__("GetJNIENVExt");


// Trampoline to invoke our logging utility and the original function.
extern "C" void InvokeVirtualFake(uint32_t, void*) __asm__("InvokeVirtualFake");
extern FUNC_INVOKE_VIRTUAL InvokeVirtualOriginal;

extern "C" void* ResolveStringFake(void*, uint32_t) __asm__("ResolveStringFake");
extern FUNC_RESOLVE_STRING ResolveStringOriginal;
extern "C" void ResolveStringProfiling(void*, uint32_t);

extern "C" void VirtualMethodCallFake(jmethodID) __asm__("VirtualMethodCallFake");
extern FUNC_VIRTUAL_METHOD_CALL VirtualMethodCallOriginal;
extern "C" void VirtualMethodCallProfiling();

#endif