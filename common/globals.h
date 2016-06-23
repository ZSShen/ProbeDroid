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
static const int kCacheSizeDWord = 4;
static const int kCacheSizeQWord = 8;

// Auxiliary symbols for inter-process communication.
static const char kSignAssign = '=';
static const char* const kKeyPathCoreLibrary = "PATH_LIB";
static const char* const kKeyPathAnalysisModule = "PATH_MODULE";
static const char* const kKeyNameMainClass = "NAME_CLASS";

// Auxiliary symbols for memory access permission.
static const char* const kPermReadExec = "r-xp";
static const char* const kPermReadWrteExec = "rwxp";

// Return value to represent if the procedure completes successfully.
enum {
    PROC_SUCC = 0,
    PROC_FAIL = 1
};

#endif