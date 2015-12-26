
#ifndef _ART_RUNTIME_STRING_INL_H_
#define _ART_RUNTIME_STRING_INL_H_


#include "mirror/string.h"


namespace art {

inline int32_t String::GetLength(const String* real)
{
    return real->count_;
}

inline const Array* String::GetCharArray(const String* real)
{
    return reinterpret_cast<const Array*>(real->hearef_char_array_);
}

}

#endif