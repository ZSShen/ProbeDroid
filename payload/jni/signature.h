#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_


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

// Cache some critical signatures used in our ProbeDroid SDK.
static const char* kNormInstrument = "org/probedroid/Instrument";

static const char* kFuncPrepareNativeBridge = "prepareNativeBridge";
static const char* kFuncOnApplicationStart = "onApplicationStart";
static const char* kFuncOnApplicationStop = "onApplicationStop";
static const char* kFuncBeforeMethodExecute = "beforeMethodExecute";
static const char* kFuncAfterMethodExecute = "afterMethodExecute";

// Cache some critical C++ function signatures exported from libart.
static const char* kIndirectReferneceTableAdd =
                    "_ZN3art22IndirectReferenceTable3AddEjPNS_6mirror6ObjectE";
static const char* kIndirectReferenceTableRemove =
                    "_ZN3art22IndirectReferenceTable6RemoveEjPv";
static const char* kThreadDecodeJObject =
                    "_ZNK3art6Thread13DecodeJObjectEP8_jobject";

#endif