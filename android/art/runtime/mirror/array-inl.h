
#ifndef _ART_RUNTIME_ARRAY_INL_H_
#define _ART_RUNTIME_ARRAY_INL_H_


#include "mirror/array.h"


namespace art {

inline int32_t Array::GetCapacity(const Array* real)
{
    return real->length_;
}

inline const uint16_t* Array::GetData(const Array* real)
{
    return &(real->first_element_);
}

}

#endif