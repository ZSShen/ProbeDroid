#ifndef _GADGET_H_
#define _GADGET_H_


#include <jni.h>


typedef void (*OriginalCall) ();


// The gadget to extract JNI handle from TLS.
extern "C" void GetJniEnv(JNIEnv**) __asm__("GetJniEnv");

// The gadget to insert an object into the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" jobject AddIndirectReference(void*, uint32_t, void*)
										__asm__("AddIndirectReference");

// The gadget to remove a reference from the designated indirect reference table.
// Note that the first argument is the pointer to art::IndirectReferenceTable.
extern "C" bool RemoveIndirectRefernce(void*, uint32_t, jobject)
										__asm__("RemoveIndirectRefernce");

// The gadget to decode the given indirect reference.
// Note that the first argument is the pointer to art::Thread.
extern "C" void* DecodeJObject(void*, jobject) __asm__("DecodeJObject");

// The original entry to IndirectReferenceTable::Add().
void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
void* g_thread_decode_jobject;

#endif