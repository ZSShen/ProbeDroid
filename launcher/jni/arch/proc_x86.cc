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

#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/mman.h>

#include "../proc.h"
#include "scoped_dl.h"
#include "log.h"
#include "stringprintf.h"


bool proc::EggHunter::RemoteMethodCall(struct user_regs_struct* reg, long* stack,
                                       size_t count_byte)
{
    bool rtn = PROC_SUCC;

    if (PokeText(reg->esp, reinterpret_cast<char*>(stack), count_byte) != PROC_SUCC)
        FINAL(EXIT, SYSERR);

    if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, reg) == -1)
        FINAL(EXIT, SYSERR);

    if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
        FINAL(EXIT, SYSERR);

    // we should receive a SIGSEGV triggered by the target app due to
    // the invalid return address.
    int status;
    if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
        FINAL(EXIT, SYSERR);
EXIT:
    return rtn;
}

bool proc::EggHunter::InjectApp(const char* lib_path, const char* module_path,
                                const char* class_name)
{
    pid_t pid_inject = getpid();

    if (func_tbl_.Resolve(pid_inject, pid_app_) != PROC_SUCC)
        return PROC_FAIL;

    // Prepare the key-value pairs for inter-process communication. Zero padding
    // is necessary to produce the text word recognized by ptrace().
    char payload[kBlahSizeMid];
    size_t len_payload = strlen(lib_path);
    strncpy(payload, lib_path, kBlahSizeMid);
    payload[len_payload++] = 0;

    size_t append = snprintf(payload + len_payload, kBlahSizeMid - len_payload,
                             "%s%c%s", kKeyPathCoreLibrary, kSignAssign, lib_path);
    len_payload += append;
    payload[len_payload++] = 0;

    append = snprintf(payload + len_payload, kBlahSizeMid - len_payload,
                      "%s%c%s", kKeyPathAnalysisModule, kSignAssign, module_path);
    len_payload += append;
    payload[len_payload++] = 0;

    append = snprintf(payload + len_payload, kBlahSizeMid - len_payload,
                      "%s%c%s", kKeyNameMainClass, kSignAssign, class_name);
    len_payload += append;
    payload[len_payload++] = 0;

    div_t count_word = div(len_payload, sizeof(long));
    size_t patch = (count_word.rem > 0)? (sizeof(long) - count_word.rem) : 0;
    for (size_t i = 0 ; i < patch ; ++i)
        payload[len_payload++] = 0;

    // Prepare the addresses of mmap() and dlopen() of the target app.
    uintptr_t addr_mmap = func_tbl_.GetMmap();
    uintptr_t addr_dlopen = func_tbl_.GetDlopen();

    bool rtn = PROC_SUCC;
    size_t count_byte;
    uintptr_t addr_blk;

    // Backup the context of the target app.
    struct user_regs_struct reg_orig;
    if (ptrace(PTRACE_GETREGS, pid_app_, nullptr, &reg_orig) == -1)
        FINAL(EXIT, SYSERR);
    struct user_regs_struct reg_modi;
    memcpy(&reg_modi, &reg_orig, sizeof(struct user_regs_struct));

    // Firstly, we force the target app to execute mmap(). The allocated
    // block will be stuffed with the path name of "libProbeDroid.so".
    // Note, to fit the calling convention,
    // param[0] stores the return address, and
    // param[6] to param[1] is the actual parameters of mmap().
    long param[kBlahSizeMid];
    param[0] = 0;
    param[1] = 0;
    param[2] = kPageSize;
    param[3] = PROT_READ | PROT_WRITE | PROT_EXEC;
    param[4] = MAP_ANONYMOUS | MAP_PRIVATE;
    param[5] = 0;
    param[6] = 0;
    count_byte = sizeof(long) * 7;

    // Set the program counter and stack pointer.
    reg_modi.eip = addr_mmap;
    reg_modi.esp -= count_byte;
    if (EggHunter::RemoteMethodCall(&reg_modi, param, count_byte) == PROC_FAIL)
        FINAL(EXIT);

    // Stuff the path name of "libProbeDroid.so" into the newly mapped space.
    if (ptrace(PTRACE_GETREGS, pid_app_, nullptr, &reg_modi) == -1)
        FINAL(EXIT, SYSERR);

    addr_blk = reg_modi.eax;
    #if __WORDSIZE == 64
    TIP() << StringPrintf("[+] mmap() successes with 0x%016lx returned.\n", addr_blk);
    #else
    TIP() << StringPrintf("[+] mmap() successes with 0x%08x returned.\n", addr_blk);
    #endif

    if (EggHunter::PokeText(addr_blk, payload, len_payload) == -1)
        FINAL(EXIT, SYSERR);

    // Secondly, we force the target app to execute dlopen().
    // Then "libProbeDroid.so" will be loaded by it.
    // Note, to fit the calling convention,
    // param[0] stores the return address, and
    // param[2] to param[1] is the actual parameters of dlopen().
    param[0] = 0;
    param[1] = addr_blk;
    param[2] = RTLD_NOW;
    count_byte = sizeof(long) * 3;

    // Set the program counter and stack pointer.
    reg_modi.eip = addr_dlopen;
    reg_modi.esp -= count_byte;
    if (EggHunter::RemoteMethodCall(&reg_modi, param, count_byte) == PROC_FAIL)
        FINAL(EXIT);
    TIP() << StringPrintf("[+] dlopen() successes.\n");

    // At this stage, we finish the library injection and should restore
    // the context of the target app.
    if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, &reg_orig) == -1)
        FINAL(EXIT, SYSERR);

    if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
        FINAL(EXIT, SYSERR);

EXIT:
    ptrace(PTRACE_DETACH, pid_app_, nullptr, nullptr);
    return rtn;
}
