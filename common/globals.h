#ifndef _GLOBALS_H_
#define _GLOBALS_H_


#include <cstdint>


typedef uint8_t byte;
typedef intptr_t word;
typedef uintptr_t uword;

static const size_t KB = 1024;
static const size_t MB = KB * KB;
static const size_t GB = KB * KB * KB;

// System page size.
static const int kPageSize = 4096;

// Auxiliary buffer size.
static const int kBlahSize = 1024;
static const int kBlahSizeMid = 512;
static const int kBlahSizeTiny = 128;

// Auxiliary symbols for inter-process communication.
static const char kSignAssign = '=';
static const char* kKeyPathCoreLibrary = "PATH_LIB";
static const char* kKeyPathAnalysisModule = "PATH_MODULE";
static const char* kKeyNameMainClass = "NAME_CLASS";

// Auxiliary symbols for memory access permission.
static const char* kPermReadExec = "r-xp";
static const char* kPermReadWrteExec = "rwxp";

// Return value to represent if the procedure completes successfully.
enum {
	PROC_SUCC = 0,
	PROC_FAIL = 1
};

#endif