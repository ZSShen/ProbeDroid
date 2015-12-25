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

#ifndef _UTIL_UTF_INL_H_
#define _UTIL_UTF_INL_H_


#include "utf.h"


inline uint16_t GetUtf16FromUtf8(const char** utf8_data_in)
{
    uint8_t one = *(*utf8_data_in)++;
    if ((one & 0x80) == 0) {
        // one-byte encoding
        return one;
    }
    // two- or three-byte encoding
    uint8_t two = *(*utf8_data_in)++;
    if ((one & 0x20) == 0) {
        // two-byte encoding
        return ((one & 0x1f) << 6) | (two & 0x3f);
    }
    // three-byte encoding
    uint8_t three = *(*utf8_data_in)++;
    return ((one & 0x0f) << 12) | ((two & 0x3f) << 6) | (three & 0x3f);
}

#endif