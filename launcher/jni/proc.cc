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
#include <iostream>
#include <iomanip>
#include <iosfwd>
#include <fstream>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/mman.h>

#include "proc.h"
#include "scoped_dl.h"
#include "log.h"
#include "except.h"
#include "stringprintf.h"


namespace proc {

static const char* kProcZygote = "zygote";
static const char* kPathLinker = "/system/bin/linker";
static const char* kPathLibc = "/system/lib/libc.so";
static const char* kNameLinker = "libdl.so";
static const char* kFuncDlopen = "dlopen";
static const char* kFuncMmap = "mmap";


#define LOG_SYSERR_AND_RETURN()                                                \
        do {                                                                   \
            PLOG(ERROR);                                                       \
            return PROC_FAIL;                                                  \
        } while (0);

#define LOG_SYSERR_AND_THROW()                                                 \
        do {                                                                   \
            PLOG(ERROR);                                                       \
            throw BadProbe();                                                  \
        } while (0);


uintptr_t FunctionTable::GetLibraryAddress(pid_t pid, const char* lib_path)
{
    char buf[kBlahSizeMid];
    sprintf(buf, "/proc/%d/maps", pid);

    std::ifstream map(buf, std::ifstream::in);
    while (map.good() && !map.eof()) {
        map.getline(buf, kBlahSizeMid);

        if (!strstr(buf, kPermReadExec))
            continue;
        if (!strstr(buf, lib_path))
            continue;
        uintptr_t addr_bgn, addr_end;
        #if __WORDSIZE == 64
        sscanf(buf, "%lx-%lx", &addr_bgn, &addr_end);
        #else
        sscanf(buf, "%x-%x", &addr_bgn, &addr_end);
        #endif
        return addr_bgn;
    }
    return 0;
}

uintptr_t FunctionTable::GetFunctionAddress(const char* dl_path, const char* func_name)
{
    ScopedDl handle(dlopen(dl_path, RTLD_LAZY));
    if (!handle.get()) {
        LOG(ERROR) << dlerror();
        return 0;
    }
    return reinterpret_cast<uintptr_t>(handle.resolve(func_name));
}

bool FunctionTable::Resolve(pid_t pid_me, pid_t pid_him)
{
    // First, resolve the address of libc and libdl loaded in the injector
    // and the target app.
    uintptr_t addr_linker_me = GetLibraryAddress(pid_me, kPathLinker);
    if (addr_linker_me == 0)
        return PROC_FAIL;
    uintptr_t addr_linker_him = GetLibraryAddress(pid_him, kPathLinker);
    if (addr_linker_him == 0)
        return PROC_FAIL;

    uintptr_t addr_libc_me = GetLibraryAddress(pid_me, kPathLibc);
    if (addr_libc_me == 0)
        return PROC_FAIL;
    uintptr_t addr_libc_him = GetLibraryAddress(pid_him, kPathLibc);
    if (addr_libc_him == 0)
        return PROC_FAIL;

    // Second, we can apply the relative offsets of the dlopen() and mmap()
    // residing in libdl and libc of the invoking app and the addresses of the
    // libdl and libc of the target app to calculate the addresses of dlopen()
    // and mmap() of the target.
    uintptr_t addr_dlopen_me = GetFunctionAddress(kNameLinker, kFuncDlopen);
    if (addr_dlopen_me == 0)
        return PROC_FAIL;
    uintptr_t addr_mmap_me = GetFunctionAddress(kPathLibc, kFuncMmap);
    if (addr_mmap_me == 0)
        return PROC_FAIL;

    dlopen_ = addr_linker_him + (addr_dlopen_me - addr_linker_me);
    mmap_ = addr_libc_him + (addr_mmap_me - addr_libc_me);
    return PROC_SUCC;
}

bool EggHunter::ExecutePs(const char* app_name, pid_t* p_pid)
{
    FILE* cmd = popen("ps", "r");
    if (!cmd)
        LOG_SYSERR_AND_RETURN();

    char buf[kBlahSizeMid];
    while (fgets(buf, kBlahSizeMid, cmd)) {
        if (!strstr(buf, app_name))
            continue;
        char* token = strtok(buf, " ");
        token = strtok(nullptr, " ");
        *p_pid = atoi(token);
    }

    if (pclose(cmd) == -1)
        LOG_SYSERR_AND_RETURN();

    return PROC_SUCC;
}

bool EggHunter::ExecuteKill(pid_t pid)
{
    char buf[kBlahSizeTiny];
    snprintf(buf, kBlahSizeTiny, "kill %d", pid);

    FILE* cmd = popen(buf, "r");
    if (!cmd)
        LOG_SYSERR_AND_RETURN();

    if (pclose(cmd) == -1)
        LOG_SYSERR_AND_RETURN();

    return PROC_SUCC;
}

void EggHunter::WaitForForkEvent(pid_t pid)
{
    while (true) {
        int status;
        if (waitpid(pid, &status, __WALL) != pid)
            LOG_SYSERR_AND_THROW()

        if ((status >> 8) == (SIGTRAP | PTRACE_EVENT_FORK << 8)) {
            // Capture the fork event and record the app PID.
            if (ptrace(PTRACE_GETEVENTMSG, pid, nullptr, &pid_app_) == -1)
                LOG_SYSERR_AND_THROW()

            if (ptrace(PTRACE_CONT, pid, nullptr, nullptr) == -1)
                LOG_SYSERR_AND_THROW()

            if (waitpid(pid_app_, &status, __WALL) != pid_app_)
                LOG_SYSERR_AND_THROW()
            break;
        }
    }
}

void EggHunter::CheckStartupCmd(const char* app_name)
{
    // Wait for the new process to finish app initialization.
    while (true) {
        // Force the newly forked process to stop at each system call entry or
        // the code points just after system call. So that we can capture the
        // timing about finishing initialization.
        if (ptrace(PTRACE_SYSCALL, pid_app_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()

        int status;
        if (waitpid(pid_app_, &status, __WALL) != pid_app_)
            LOG_SYSERR_AND_THROW()

        char buf[kBlahSizeMid];
        sprintf(buf, "/proc/%d/cmdline", pid_app_);

        std::ifstream cmd(buf, std::ifstream::in);
        if (cmd.good())
            cmd.getline(buf, kBlahSizeMid);
        if (strstr(buf, app_name)) {
            TIP() << StringPrintf("[+] Capture the target app %s(%d).\n", buf, pid_app_);
            break;
        }
    }
}

bool EggHunter::PokeTextInApp(uintptr_t addr_txt, const char* buf, size_t count_byte)
{
    size_t count_wrt = 0;
    while (count_wrt < count_byte) {
        long word;
        memcpy(&word, buf + count_wrt, sizeof(word));
        if (ptrace(PTRACE_POKETEXT, pid_app_, addr_txt + count_wrt, word) == -1)
            return PROC_FAIL;
        count_wrt += sizeof(word);
    }
    return PROC_SUCC;
}

bool EggHunter::PeekTextInApp(uintptr_t addr_txt, char* buf, size_t count_byte)
{
    long* slide = reinterpret_cast<long*>(buf);
    size_t count_read = 0, idx = 0;
    while (count_read < count_byte) {
        long word = ptrace(PTRACE_PEEKTEXT, pid_app_, addr_txt + count_read, nullptr);
        if (word == -1)
            return PROC_FAIL;
        count_read += sizeof(word);
        slide[idx++] = word;
    }
    return PROC_SUCC;
}

bool EggHunter::Initialize(const char* app_name)
{
    pid_t pid;

    // Capture the zygote pid.
    pid = -1;
    if (ExecutePs(kProcZygote, &pid) == PROC_FAIL)
        return PROC_FAIL;
    if (pid == -1)
        return PROC_FAIL;
    pid_zygote_ = pid;

    // Before instrumentation, kill the target app if it is still active now.
    pid = -1;
    if (ExecutePs(app_name, &pid) == PROC_FAIL)
        return PROC_FAIL;
    if (pid != -1) {
        if (ExecuteKill(pid) == PROC_FAIL)
            return PROC_FAIL;
    }

    return PROC_SUCC;
}

bool EggHunter::CaptureApp(const char* app_name)
{
    bool rtn = PROC_SUCC;
    try {
        // First, we need to attach to the zygote process and wait for the
        // target app to be forked after our triggering.
        if (ptrace(PTRACE_ATTACH, pid_zygote_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()

        int status;
        if (waitpid(pid_zygote_, &status, WUNTRACED) != pid_zygote_)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETOPTIONS, pid_zygote_, 1, PTRACE_O_TRACEFORK) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_zygote_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()

        // Second, we enter the polling loop to wait for the target app. When
        // we catch the fork event fired by zygote, we should verify if the
        // newly forked app is our target by examining the startup command. If
        // so, we PROC_SUCCfully get the goal and should release zygote.
        WaitForForkEvent(pid_zygote_);
        CheckStartupCmd(app_name);
    } catch (BadProbe& e) {
        rtn = PROC_FAIL;
        if (pid_app_ != 0)
            ptrace(PTRACE_DETACH, pid_app_, nullptr, nullptr);
    }

    ptrace(PTRACE_DETACH, pid_zygote_, nullptr, nullptr);
    return rtn;
}

bool EggHunter::InjectApp(const char* lib_path, const char* module_path,
                          const char* class_name)
{
    pid_t pid_inject = getpid();

    if (func_tbl_.Resolve(pid_inject, pid_app_) != PROC_SUCC)
        return PROC_FAIL;

    // Prepare the library pathname for dlopen() and the key-value pairs for
    // inter-process communication. Zero padding is necessary to produce the
    // text word recognized by ptrace().
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
    try {
        // Backup the context of the target app.
        struct user_regs_struct reg_orig;
        if (ptrace(PTRACE_GETREGS, pid_app_, nullptr, &reg_orig) == -1)
            LOG_SYSERR_AND_THROW();

        // Firstly, we force the target app to execute mmap(). The allocated
        // block will be stuffed with the pathname of the hooking library.
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
        size_t count_byte = sizeof(long) * 7;

        struct user_regs_struct reg_modi;
        memcpy(&reg_modi, &reg_orig, sizeof(struct user_regs_struct));
        reg_modi.eip = addr_mmap;
        reg_modi.esp -= count_byte;

        if (PokeTextInApp(reg_modi.esp, reinterpret_cast<char*>(param),
                          count_byte) != PROC_SUCC)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()

        // The injector will receive a SIGSEGV triggered by the target app due to
        // the invalid return address.
        int status;
        if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
            LOG_SYSERR_AND_THROW()

        // Stuff the pathname of the hooking library into the newly mapped
        // memory segment.
        if (ptrace(PTRACE_GETREGS, pid_app_, nullptr, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        uintptr_t addr_blk = reg_modi.eax;
        #if __WORDSIZE == 64
        TIP() << StringPrintf("[+] mmap() successes with 0x%016lx returned.\n", addr_blk);
        #else
        TIP() << StringPrintf("[+] mmap() successes with 0x%08x returned.\n", addr_blk);
        #endif

        if (PokeTextInApp(addr_blk, payload, len_payload) == -1)
            LOG_SYSERR_AND_THROW()

        // Secondly, we force the target app to execute dlopen(). Then our hooking
        // library will be loaded by target.
        // Note, to fit the calling convention,
        // param[0] stores the return address, and
        // param[2] to param[1] is the actual parameters of dlopen().
        param[0] = 0;
        param[1] = addr_blk;
        param[2] = RTLD_NOW;
        count_byte = sizeof(long) * 3;

        reg_modi.eip = addr_dlopen;
        reg_modi.esp -= count_byte;

        if (PokeTextInApp(reg_modi.esp, reinterpret_cast<char*>(param),
                          count_byte) != PROC_SUCC)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()

        // The injector will receive a SIGSEGV triggered by target app due to
        // invalid return address.
        if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
            LOG_SYSERR_AND_THROW()
        TIP() << StringPrintf("[+] dlopen() successes.\n");

        // At this stage, we finish the library injection and should restore
        // the context of the target app.
        if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, &reg_orig) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
            LOG_SYSERR_AND_THROW()
    } catch (BadProbe& e) {
        rtn = PROC_FAIL;
        if (pid_app_ != 0)
            ptrace(PTRACE_DETACH, pid_app_, nullptr, nullptr);
    }

    ptrace(PTRACE_DETACH, pid_app_, nullptr, nullptr);
    return rtn;
}

void EggHunter::Hunt(const char* app_name, const char* lib_path,
                     const char* module_path, const char* class_name)
{
    if (Initialize(app_name) != PROC_SUCC)
        return;
    if (CaptureApp(app_name) != PROC_SUCC)
        return;
    if (InjectApp(lib_path, module_path, class_name) != PROC_SUCC)
        return;
    TIP() << StringPrintf("[+] %s is successfully injected.\n", lib_path);
}

}