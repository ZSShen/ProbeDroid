#ifndef _UTIL_GLOBALS_H_
#define _UTIL_GLOBALS_H_


#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <utility>
#include <iostream>
#include <iomanip>
#include <iosfwd>
#include <sstream>
#include <fstream>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <getopt.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>


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