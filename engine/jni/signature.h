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

#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include <vector>


// Cache some commonly used Java signatures for method invocation.
static const char* const kNormClassLoader = "java/lang/ClassLoader";
static const char* const kNormDexClassLoader = "dalvik/system/DexClassLoader";
static const char* const kNormDexFile = "dalvik/system/DexFile";
static const char* const kNormObject = "java/lang/Object";
static const char* const kNormBooleanObject = "java/lang/Boolean";
static const char* const kNormByteObject = "java/lang/Byte";
static const char* const kNormCharObject = "java/lang/Character";
static const char* const kNormShortObject = "java/lang/Short";
static const char* const kNormIntObject = "java/lang/Integer";
static const char* const kNormLongObject = "java/lang/Long";
static const char* const kNormFloatObject = "java/lang/Float";
static const char* const kNormDoubleObject = "java/lang/Double";
static const char* const kNormIllegalArgument = "java/lang/IllegalArgumentException";
static const char* const kNormClassNotFound = "java/lang/ClassNotFoundException";
static const char* const kNormNoSuchMethod = "java/lang/NoSuchMethodException";

static const char* const kFuncConstructor = "<init>";
static const char* const kFuncGetSystemClassLoader = "getSystemClassLoader";
static const char* const kFuncToString = "toString";
static const char* const kFuncLoadClass = "loadClass";
static const char* const kFuncLoadDex = "loadDex";
static const char* const kFuncEntries = "entries";
static const char* const kFuncHasMoreElements = "hasMoreElements";
static const char* const kFuncNextElement = "nextElement";
static const char* const kFuncClose = "close";
static const char* const kFuncBooleanValue = "booleanValue";
static const char* const kFuncByteValue = "byteValue";
static const char* const kFuncCharValue = "charValue";
static const char* const kFuncShortValue = "shortValue";
static const char* const kFuncIntValue = "intValue";
static const char* const kFuncLongValue = "longValue";
static const char* const kFuncFloatValue = "floatValue";
static const char* const kFuncDoubleValue = "doubleValue";

static const char* const kSigClassLoader = "Ljava/lang/ClassLoader;";
static const char* const kSigDexClassLoader = "Ldalvik/system/DexClassLoader;";
static const char* const kSigString = "Ljava/lang/String;";
static const char* const kSigClass = "Ljava/lang/Class;";
static const char* const kSigDexFile = "Ldalvik/system/DexFile;";

static const char* const kSigObjectObject = "Ljava/lang/Object;";
static const char* const kSigBooleanObject = "Ljava/lang/Boolean;";
static const char* const kSigByteObject = "Ljava/lang/Byte;";
static const char* const kSigCharObject = "Ljava/lang/Character;";
static const char* const kSigShortObject = "Ljava/lang/Short;";
static const char* const kSigIntObject = "Ljava/lang/Integer;";
static const char* const kSigLongObject = "Ljava/lang/Long;";
static const char* const kSigFloatObject = "Ljava/lang/Float;";
static const char* const kSigDoubleObject = "Ljava/lang/Double;";
static const char* const kSigEnumeration = "Ljava/util/Enumeration;";

static const char kSigVoid = 'V';
static const char kSigBoolean = 'Z';
static const char kSigByte = 'B';
static const char kSigChar = 'C';
static const char kSigShort = 'S';
static const char kSigInt = 'I';
static const char kSigLong = 'J';
static const char kSigFloat = 'F';
static const char kSigDouble = 'D';
static const char kSigObject = 'L';
static const char kSigArray = '[';

static const char kTypeVoid = 0;
static const char kTypeBoolean = 1;
static const char kTypeByte = 2;
static const char kTypeChar = 3;
static const char kTypeShort = 4;
static const char kTypeInt = 5;
static const char kTypeLong = 6;
static const char kTypeFloat = 7;
static const char kTypeDouble = 8;
static const char kTypeObject = 9;

static const uint32_t kWidthDword = 1;
static const uint32_t kWidthQword = 2;

static const char kDeliSlash = '/';
static const char kDeliDot = '.';
static const char kDeliLeftInput = '(';
static const char kDeliRightInput = ')';
static const char kDeliObjectTail = ';';

// Cache some critical signatures used in our ProbeDroid SDK.
static const char* const kNormInstrument = "org/probedroid/Instrument";

static const char* const kPkgInstrument = "org.probedroid.Instrument";
static const char* const kPrefixAndroidSupport = "android.support";

static const char* const kFuncPrepareNativeBridge = "prepareNativeBridge";
static const char* const kFuncOnApplicationStart = "onApplicationStart";
static const char* const kFuncOnApplicationStop = "onApplicationStop";
static const char* const kFuncBeforeMethodExecute = "beforeMethodExecute";
static const char* const kFuncAfterMethodExecute = "afterMethodExecute";

static const char* const kFieldInterceptBefore = "mInterceptBefore";
static const char* const kFieldInterceptAfter = "mInterceptAfter";
static const char* const kFieldPathOutputDirectory = "mPathOutputDirectory";

// Cache some critical C++ function signatures exported from libart.
static const char* const kGetCreatedJavaVMs = "JNI_GetCreatedJavaVMs";
static const char* const kIndirectReferneceTableAdd =
                    "_ZN3art22IndirectReferenceTable3AddEjPNS_6mirror6ObjectE";
static const char* const kIndirectReferenceTableRemove =
                    "_ZN3art22IndirectReferenceTable6RemoveEjPv";
static const char* const kThreadDecodeJObject =
                    "_ZNK3art6Thread13DecodeJObjectEP8_jobject";
static const char* const kCreateInternalStackTrace =
"_ZNK3art6Thread24CreateInternalStackTraceILb0EEEP8_jobjectRKNS_33ScopedObjectAccessAlreadyRunnableE";

extern const uint32_t kNoData;
extern const uint32_t kDwordInt;
extern const uint32_t kDwordFloat;
extern const uint32_t kQwordLong;
extern const uint32_t kQwordDouble;


class MethodSignatureParser
{
  public:
    MethodSignatureParser(const char* signature)
     : output_(kTypeVoid),
       signature_(signature),
       inputs_()
    {}

    void Parse();

    const std::vector<char>& GetInputType()
    {
        return inputs_;
    }

    char GetOutputType()
    {
        return output_;
    }

  private:
    char output_;
    const char* signature_;
    std::vector<char> inputs_;
};

#endif