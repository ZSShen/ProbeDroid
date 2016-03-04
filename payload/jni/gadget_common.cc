#include "gadget.h"


// The cached Java VM handle.
JavaVM* g_jvm;

// The original entry to IndirectReferenceTable::Add().
void* g_indirect_reference_table_add;

// The original entry to IndirectReferneceTable::Remove().
void* g_indirect_reference_table_remove;

// The original entry to Thread::DecodeJObject().
void* g_thread_decode_jobject;

// The original entry to the loadClass() quick compiled code.
void* g_load_class_quick_compiled;
