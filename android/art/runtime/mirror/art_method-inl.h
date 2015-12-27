
#ifndef _ART_RUNTIME_ART_METHOD_INL_H_
#define _ART_RUNTIME_ART_METHOD_INL_H_


#include "mirror/art_method.h"


namespace art {

inline uint64_t ArtMethod::GetEntryPointFromQuickCompiledCode(const ArtMethod* real)
{
    return real->entry_point_from_quick_compiled_code_;
}

inline void ArtMethod::SetEntryPointFromQuickCompiledCode(ArtMethod* real,
                                                          uint64_t entry)
{
    real->entry_point_from_quick_compiled_code_ = entry;
}

}


#endif