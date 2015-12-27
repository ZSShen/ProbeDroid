
#ifndef _ART_RUNTIME_ART_METHOD_H_
#define _ART_RUNTIME_ART_METHOD_H_


#include "mirror/object.h"


namespace art {

class ArtMethod : public Object
{
  public:
    static uint64_t GetEntryPointFromQuickCompiledCode(const ArtMethod*);
    static void SetEntryPointFromQuickCompiledCode(ArtMethod*, uint64_t);

    friend uint64_t GetEntryPointFromQuickCompiledCode(const ArtMethod*);
    friend void SetEntryPointFromQuickCompiledCode(ArtMethod*, uint64_t);

  private:
    // The class we are a part of.
    uint32_t heapref_declaring_class_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    uint32_t heapref_dex_cache_resolved_methods_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    uint32_t heapref_dex_cache_resolved_types_;

    // Short cuts to declaring_class_->dex_cache_ member for fast compiled code access.
    uint32_t heapref_dex_cache_resolved_strings_;

    // Method dispatch from the interpreter invokes this pointer which may cause
    // a bridge into compiled code.
    uint64_t entry_point_from_interpreter_;

    // Pointer to JNI function registered to this method, or a function to resolve
    // the JNI function.
    uint64_t entry_point_from_jni_;

    // Method dispatch from portable compiled code invokes this pointer which may
    // cause bridging into quick compiled code or the interpreter.
#if defined(ART_USE_PORTABLE_COMPILER)
    uint64_t entry_point_from_portable_compiled_code_;
#endif

    // Method dispatch from quick compiled code invokes this pointer which may cause
    // bridging into portable compiled code or the interpreter.
    uint64_t entry_point_from_quick_compiled_code_;

    // Pointer to a data structure created by the compiler and used by the garbage
    // collector to determine which registers hold live references to objects within
    // the heap. Keyed by native PC offsets for the quick compiler and dex PCs
    // for the portable.
    uint64_t gc_map_;

    // Access flags; low 16 bits are defined by spec.
    uint32_t access_flags_;

    /* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */
    // Offset to the CodeItem.
    uint32_t dex_code_item_offset_;

    // Index into method_ids of the dex file associated with this method.
    uint32_t dex_method_index_;
    /* End of dex file fields. */

    // Entry within a dispatch table for this method. For static/direct methods
    // the index is into the declaringClass.directMethods, for virtual methods
    // the vtable and for interface methods the ifTable.
    uint32_t method_index_;
};

}

#endif