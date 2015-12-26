
#ifndef _ART_RUNTIME_ARRAY_H_
#define _ART_RUNTIME_ARRAY_H_


#include "mirror/object.h"


namespace art {

class Array : public Object
{
  public:
    static int32_t GetCapacity(const Array*);
    static const uint16_t* GetData(const Array*);

    friend int32_t GetCapacity(const Array*);
    friend const uint16_t* GetData(const Array*);

  private:
    int32_t length_;
    uint16_t first_element_;
};

}

#endif