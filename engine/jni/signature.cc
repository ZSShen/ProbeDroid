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
