#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include <vector>


// Cache some commonly used Java signatures for method invocation.
static const char* kNormClassLoader = "java/lang/ClassLoader";
static const char* kNormDexClassLoader = "dalvik/system/DexClassLoader";
static const char* kNormDexFile = "dalvik/system/DexFile";
static const char* kNormObject = "java/lang/Object";
static const char* kNormBooleanObject = "java/lang/Boolean";
static const char* kNormByteObject = "java/lang/Byte";
static const char* kNormCharObject = "java/lang/Character";
static const char* kNormShortObject = "java/lang/Short";
static const char* kNormIntObject = "java/lang/Integer";
static const char* kNormLongObject = "java/lang/Long";
static const char* kNormFloatObject = "java/lang/Float";
static const char* kNormDoubleObject = "java/lang/Double";
static const char* kNormIllegalArgument = "java/lang/IllegalArgumentException";
static const char* kNormClassNotFound = "java/lang/ClassNotFoundException";
static const char* kNormNoSuchMethod = "java/lang/NoSuchMethodException";

static const char* kFuncConstructor = "<init>";
static const char* kFuncGetSystemClassLoader = "getSystemClassLoader";
static const char* kFuncToString = "toString";
static const char* kFuncLoadClass = "loadClass";
static const char* kFuncLoadDex = "loadDex";
static const char* kFuncEntries = "entries";
static const char* kFuncHasMoreElements = "hasMoreElements";
static const char* kFuncNextElement = "nextElement";
static const char* kFuncBooleanValue = "booleanValue";
static const char* kFuncByteValue = "byteValue";
static const char* kFuncCharValue = "charValue";
static const char* kFuncShortValue = "shortValue";
static const char* kFuncIntValue = "intValue";
static const char* kFuncLongValue = "longValue";
static const char* kFuncFloatValue = "floatValue";
static const char* kFuncDoubleValue = "doubleValue";

static const char* kSigClassLoader = "Ljava/lang/ClassLoader;";
static const char* kSigDexClassLoader = "Ldalvik/system/DexClassLoader;";
static const char* kSigString = "Ljava/lang/String;";
static const char* kSigClass = "Ljava/lang/Class;";
static const char* kSigDexFile = "Ldalvik/system/DexFile;";

static const char* kSigObjectObject = "Ljava/lang/Object;";
static const char* kSigBooleanObject = "Ljava/lang/Boolean;";
static const char* kSigByteObject = "Ljava/lang/Byte;";
static const char* kSigCharObject = "Ljava/lang/Character;";
static const char* kSigShortObject = "Ljava/lang/Short;";
static const char* kSigIntObject = "Ljava/lang/Integer;";
static const char* kSigLongObject = "Ljava/lang/Long;";
static const char* kSigFloatObject = "Ljava/lang/Float;";
static const char* kSigDoubleObject = "Ljava/lang/Double;";
static const char* kSigEnumeration = "Ljava/util/Enumeration;";

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
static const char* kNormInstrument = "org/probedroid/Instrument";

static const char* kFuncPrepareNativeBridge = "prepareNativeBridge";
static const char* kFuncOnApplicationStart = "onApplicationStart";
static const char* kFuncOnApplicationStop = "onApplicationStop";
static const char* kFuncBeforeMethodExecute = "beforeMethodExecute";
static const char* kFuncAfterMethodExecute = "afterMethodExecute";

static const char* kFieldInterceptBefore = "mInterceptBefore";
static const char* kFieldInterceptAfter = "mInterceptAfter";

// Cache some critical C++ function signatures exported from libart.
static const char* kIndirectReferneceTableAdd =
                    "_ZN3art22IndirectReferenceTable3AddEjPNS_6mirror6ObjectE";
static const char* kIndirectReferenceTableRemove =
                    "_ZN3art22IndirectReferenceTable6RemoveEjPv";
static const char* kThreadDecodeJObject =
                    "_ZNK3art6Thread13DecodeJObjectEP8_jobject";

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