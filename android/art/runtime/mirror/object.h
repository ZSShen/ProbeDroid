
#ifndef _ART_RUNTIME_OBJECT_H_
#define _ART_RUNTIME_OBJECT_H_


#include <stdint.h>
#include <string.h>
#include <string>

namespace art {

class Object
{
  private:
    // The class representing the type of the object.
    uint32_t heapref_class_;

    // Monitor and hash code information.
    uint32_t monitor_;
};

}

#endif