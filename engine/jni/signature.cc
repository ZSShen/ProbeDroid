/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

#include <cstdint>

#include "signature.h"


const uint32_t kNoData = 0x0;
const uint32_t kDwordInt = 0x1;
const uint32_t kDwordFloat = 0x2;
const uint32_t kQwordLong = 0x10 ^ 0x1;
const uint32_t kQwordDouble = 0x10 ^ 0x2;


void MethodSignatureParser::Parse()
{
    // Parse the input types.
    char* ofst = const_cast<char*>(signature_);
    char ch;
    do {
        ++ofst;
        ch = *ofst;
        switch (ch) {
            case kSigBoolean:
                inputs_.push_back(kTypeBoolean);
                break;
            case kSigByte:
                inputs_.push_back(kTypeByte);
                break;
            case kSigChar:
                inputs_.push_back(kTypeChar);
                break;
            case kSigShort:
                inputs_.push_back(kTypeShort);
                break;
            case kSigInt:
                inputs_.push_back(kTypeInt);
                break;
            case kSigLong:
                inputs_.push_back(kTypeLong);
                break;
            case kSigFloat:
                inputs_.push_back(kTypeFloat);
                break;
            case kSigDouble:
                inputs_.push_back(kTypeDouble);
                break;
            case kSigObject:
                inputs_.push_back(kTypeObject);
                while (*ofst != kDeliObjectTail)
                    ++ofst;
                break;
            case kSigArray:
                inputs_.push_back(kTypeObject);
                while (*ofst == kSigArray)
                    ++ofst;
                if (*ofst == kSigObject) {
                    while (*ofst != kDeliObjectTail)
                        ++ofst;
                }
                break;
            default:
                break;
        }
    } while (ch != kDeliRightInput);

    // Parse the output type.
    ++ofst;
    switch (*ofst) {
        case kSigVoid:
            break;
        case kSigBoolean:
            output_ = kTypeBoolean;
            break;
        case kSigByte:
            output_ = kTypeByte;
            break;
        case kSigChar:
            output_ = kTypeChar;
            break;
        case kSigShort:
            output_ = kTypeShort;
            break;
        case kSigInt:
            output_ = kTypeInt;
            break;
        case kSigLong:
            output_ = kTypeLong;
            break;
        case kSigFloat:
            output_ = kTypeFloat;
            break;
        case kSigDouble:
            output_ = kTypeDouble;
            break;
        case kSigObject:
        case kSigArray:
            output_ = kTypeObject;
            break;
    }
}
