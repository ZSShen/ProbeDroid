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