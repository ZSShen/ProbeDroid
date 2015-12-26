
#ifndef _ART_RUNTIME_OBJECT_H_
#define _ART_RUNTIME_OBJECT_H_


#include <stdint.h>


namespace art {

class Object
{
  private:
    // The class representing the type of the object.
    // We just simplify the heap reference object to an address.
    uint32_t heapref_class_;

    // Monitor and hash code information.
    uint32_t monitor_;
};

}

#endif