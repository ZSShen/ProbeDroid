#ifndef _UTIL_GLOBALS_H_
#define _UTIL_GLOBALS_H_


#include <cstdint>


typedef uint8_t byte;
typedef intptr_t word;
typedef uintptr_t uword;

static constexpr size_t KB = 1024;
static constexpr size_t MB = KB * KB;
static constexpr size_t GB = KB * KB * KB;

// System page size.
static constexpr int kPageSize = 4096;

// Auxiliary buffer size.
static constexpr int kBlahSize = 1024;
static constexpr int kBlahSizeMid = 512;
static constexpr int kBlahSizeTiny = 128;

// Auxiliary symbols for inter-process communication.
static const char kSignAssign = '=';
static const char* kKeyPathCoreLibrary = "PATH_LIB";
static const char* kKeyPathAnalysisModule = "PATH_MODULE";
static const char* kKeyNameMainClass = "NAME_CLASS";

// Auxiliary symbols for memory access permission.
static const char* kPermReadExec = "r-xp";
static const char* kPermReadWrteExec = "rwxp";

#endif