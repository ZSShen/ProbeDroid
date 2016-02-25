#ifndef _GADGET_H_
#define _GADGET_H_


#include <jni.h>
#include <cstdint>


// The gadget to extract JNI handle from TLS.
extern "C" void GetJniEnv(JNIEnv**) __asm__("GetJniEnv");

// The gadget to insert an object into the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" jobject AddIndirectReference(void*, uint32_t, void*)
										__asm__("AddIndirectReference");

// The gadget to remove a reference from the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" bool RemoveIndirectReference(void*, uint32_t, jobject)
										__asm__("RemoveIndirectReference");

// The gadget to decode the given indirect reference.
// Note that the first argument is the pointer to art::Thread.
extern "C" void* DecodeJObject(void*, jobject) __asm__("DecodeJObject");

// The trampoline to the hook gadget compiler.
extern "C" void* CompileHookGadgetTrampoline()
										__asm__("CompileHookGadgetTrampoline");

// The compiler which sets all the hooking gadgets towards user designated
// Java methods for instrumentation.
extern "C" void* CompileHookGadget(void*, void*, void*, void*, void*);


// The cached Java VM handle.
extern JavaVM* g_jvm;

// The original entry to IndirectReferenceTable::Add().
extern void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
extern void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
extern void* g_thread_decode_jobject;

// The original entry to the loadClass() quick compiled code.
extern void* g_load_class_quick_compiled;

#endif