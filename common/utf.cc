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

#include "utf.h"
#include "utf-inl.h"


size_t CountModifiedUtf8Chars(const char* utf8)
{
    size_t len = 0;
    int ic;
    while ((ic = *utf8++) != '\0') {
        len++;
        if ((ic & 0x80) == 0) {
            // one-byte encoding
            continue;
        }
        // two- or three-byte encoding
        utf8++;
        if ((ic & 0x20) == 0) {
            // two-byte encoding
            continue;
        }
        // three-byte encoding
        utf8++;
    }
    return len;
}

void ConvertUtf16ToModifiedUtf8(char* utf8_out, const uint16_t* utf16_in,
                                size_t char_count)
{
    while (char_count--) {
        uint16_t ch = *utf16_in++;
        if (ch > 0 && ch <= 0x7f)
            *utf8_out++ = ch;
        else {
            if (ch > 0x07ff) {
                *utf8_out++ = (ch >> 12) | 0xe0;
                *utf8_out++ = ((ch >> 6) & 0x3f) | 0x80;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            } else /*(ch > 0x7f || ch == 0)*/ {
                *utf8_out++ = (ch >> 6) | 0xc0;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            }
        }
    }
}
