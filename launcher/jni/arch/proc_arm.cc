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

#include "../proc.h"
#include "scoped_dl.h"
#include "log.h"
#include "stringprintf.h"


namespace proc {

// The number of cache registers to store function call parameters.
static const size_t kCacheRegCount = 4;

// The bit mask to check for ARM or Thumb mode.
static const uint32_t kModeMask = 0x1 << 5;


bool EggHunter::RemoteFunctionCall(struct pt_regs* reg, uintptr_t addr_func,
                            long* param, size_t count_word, uintptr_t* p_ret)
{
    bool rtn = PROC_SUCC;

    // The first parameter is the invalid return address which won't be used
    // in the ARM related architecture.
    ++param;
    --count_word;

    // Overwrite the target cache registers with the first 4 parameters.
    size_t idx_word = 0;
    while (idx_word < count_word && idx_word < kCacheRegCount) {
        reg->uregs[idx_word] = param[idx_word];
        ++idx_word;
    }

    if (idx_word < count_word) {
        count_word -= kCacheRegCount;
        size_t count_byte = count_word * sizeof(long);

        // Set the new stack pointer.
        reg->ARM_sp -= count_byte;

        // Overwrite the target stack content with the remained parameters.
        if (PokeText(reg->ARM_sp, reinterpret_cast<char*>(param + kCacheRegCount),
                    count_byte) != PROC_SUCC)
            FINAL(EXIT, SYSERR);
    }

    // Set the new program counter.
    reg->ARM_pc = addr_func;
    if (reg->ARM_pc & 0x1) {
        // Thumb mode. All the instructions should be half-word aligned.
        reg->ARM_pc &= ~0x1;
        reg->ARM_cpsr |= kModeMask;
    } else
        // ARM mode
        reg->ARM_cpsr &= ~kModeMask;

    // Set the invalid return address.
    reg->ARM_lr = 0;

    // Overwrite the target register set.
    if (ptrace(PTRACE_SETREGS, pid_app_, nullptr, reg) == -1)
        FINAL(EXIT, SYSERR);

    // Invoke the remote function.
    if (ptrace(PTRACE_CONT, pid_app_, nullptr, nullptr) == -1)
        FINAL(EXIT, SYSERR);

    // We should receive a SIGSEGV triggered by the target app due to
    // the invalid return address.
    int status;
    if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
        FINAL(EXIT, SYSERR);

    if (p_ret) {
        if (ptrace(PTRACE_GETREGS, pid_app_, nullptr, reg) == -1)
            FINAL(EXIT, SYSERR);
        *p_ret = reg->ARM_r0;
    }

EXIT:
    return rtn;
}

}
