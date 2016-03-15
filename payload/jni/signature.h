#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include <vector>


// Cache some commonly used Java signatures for method invocation.
static const char* kNormClassLoader = "java/lang/ClassLoader";
static const char* kNormDexClassLoader = "dalvik/system/DexClassLoader";
static const char* kNormObject = "java/lang/Object";
static const char* kNormDexFile = "dalvik/system/DexFile";

static const char* kFuncConstructor = "<init>";
static const char* kFuncGetSystemClassLoader = "getSystemClassLoader";
static const char* kFuncToString = "toString";
static const char* kFuncLoadClass = "loadClass";
static const char* kFuncLoadDex = "loadDex";
static const char* kFuncEntries = "entries";
static const char* kFuncHasMoreElements = "hasMoreElements";
static const char* kFuncNextElement = "nextElement";

static const char* kSigClassLoader = "Ljava/lang/ClassLoader;";
static const char* kSigDexClassLoader = "Ldalvik/system/DexClassLoader;";
static const char* kSigString = "Ljava/lang/String;";
static const char* kSigClass = "Ljava/lang/Class;";
static const char* kSigDexFile = "Ldalvik/system/DexFile;";
static const char* kSigEnumeration = "Ljava/util/Enumeration;";
static const char* kSigObjectLong = "Ljava/lang/Object;";
static const char* kSigIllegalArgument = "Ljava/lang/IllegalArgumentException;";
static const char* kSigClassNotFound = "Ljava/lang/ClassNotFoundException;";
static const char* kSigNoSuchMethod = "Ljava/lang/NoSuchMethodException";

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
static const char kTypeArray = 16;

static const uint32_t kWidthByte = 1;
static const uint32_t kWidthWord = 2;
static const uint32_t kWidthDword = 4;
static const uint32_t kWidthQword = 8;

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
extern const uint32_t kDowrdFloat;
extern const uint32_t kQWordLong;
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