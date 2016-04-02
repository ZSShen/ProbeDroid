/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ART_RUNTIME_STRING_H_
#define _ART_RUNTIME_STRING_H_


#include "mirror/object.h"
#include "mirror/array.h"


namespace art {

class String : public Object
{
  public:
    static int32_t GetLength(const String*);
    static const Array* GetCharArray(const String*);
    static std::string ToModifiedUtf8(const String*);

    friend int32_t GetLength(const String*);
    friend const Array* GetCharArray(const String*);

  private:
    // The reference to the object storing string content.
    // We just simplify the heap reference object to an address.
    uint32_t hearef_char_array_;

    int32_t count_;
    uint32_t hash_code_;
    int32_t offset_;
};

}

#endif