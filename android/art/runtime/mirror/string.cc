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

#include "mirror/string-inl.h"
#include "mirror/array-inl.h"
#include "utf.h"


namespace art {

std::string String::ToModifiedUtf8(const String *real)
{
    size_t str_len = static_cast<size_t>(GetLength(real));
    const Array* char_array = GetCharArray(real);

    size_t array_capacity = static_cast<size_t>(Array::GetCapacity(char_array));
    const uint16_t* utf16_in = Array::GetData(char_array);

    char utf8_out[array_capacity];
    memset(utf8_out, 0, sizeof(char) * array_capacity);
    ConvertUtf16ToModifiedUtf8(utf8_out, utf16_in, str_len);
    return std::string(utf8_out);
}

}