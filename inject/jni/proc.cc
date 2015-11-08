#include <proc.h>

namespace proc {

using namespace util;

#define PATH_LINKER             "/system/bin/linker"
#define PATH_LIBC               "/system/lib/libc.so"
#define NAME_LINKER             "libdl.so"
#define FUNC_DLOPEN             "dlopen"
#define FUNC_MMAP               "mmap"
#define PERM_READ_EXEC          "r-xp"

#define LOG_SYSERR_AND_THROW()                                                 \
        do {                                                                   \
            ERROR("[-] %s", strerror(errno));                                  \
            throw BadProbe();                                                  \
        } while (0);


uint32_t FunctionTable::GetLibBgnAddr(pid_t pid, const char* sz_lib)
{
    char buf[SIZE_MID_BLAH];
    sprintf(buf, "/proc/%d/maps", pid);

    std::ifstream map(buf, std::ifstream::in);
    while (map.good() && !map.eof()) {
        map.getline(buf, SIZE_MID_BLAH);
        if (!strstr(buf, PERM_READ_EXEC))
            continue;
        if (!strstr(buf, sz_lib))
            continue;

        uint32_t addr_bgn, addr_end;
        sscanf(buf, "%x-%x", &addr_bgn, &addr_end);
        return addr_bgn;
    }
    return 0;
}

uint32_t FunctionTable::GetFuncBgnAddr(const char* sz_path, const char* sz_func)
{
    uint32_t addr_func = 0;
    try {
        DlHandle hdle(sz_path, RTLD_LAZY);
        addr_func = hdle.ResolveSymbol(sz_func);
    } catch (BadSharedObject& e) {
        ERROR("[-] %s", dlerror());
    }
    return addr_func;
}

int32_t FunctionTable::Resolve(pid_t pid_me, pid_t pid_him)
{
    // First, resolve the address of libc and libdl loaded in the invoking app
    // and the target app.
    uint32_t addr_linker_me = GetLibBgnAddr(pid_me, PATH_LINKER);
    if (!addr_linker_me)
        return FAIL;
    uint32_t addr_linker_him = GetLibBgnAddr(pid_him, PATH_LINKER);
    if (!addr_linker_him)
        return FAIL;

    uint32_t addr_libc_me = GetLibBgnAddr(pid_me, PATH_LIBC);
    if (!addr_libc_me)
        return FAIL;
    uint32_t addr_libc_him = GetLibBgnAddr(pid_him, PATH_LIBC);
    if (!addr_libc_him)
        return FAIL;

    // Second, we can apply the relative offsets of the dlopen() and mmap()
    // residing in libdl and libc of the invoking app and the addresses of the
    // libdl and libc of the target app to calculate the addresses of dlopen()
    // and mmap() of the target.
    uint32_t addr_dlopen_me = GetFuncBgnAddr(NAME_LINKER, FUNC_DLOPEN);
    if (!addr_dlopen_me)
        return FAIL;
    dlopen_ = addr_linker_him + (addr_dlopen_me - addr_linker_me);

    uint32_t addr_mmap_me = GetFuncBgnAddr(PATH_LIBC, FUNC_MMAP);
    if (!addr_mmap_me)
        return FAIL;
    mmap_ = addr_libc_him + (addr_mmap_me - addr_libc_me);

    std::cout << mmap_ << " " << dlopen_ << std::endl;
    return SUCC;
}

void EggHunter::WaitForForkEvent(pid_t pid)
{
    while (true) {
        int32_t status;
        if (waitpid(pid, &status, __WALL) != pid)
            LOG_SYSERR_AND_THROW()

        if ((status >> 8) == (SIGTRAP | PTRACE_EVENT_FORK << 8)) {
            // Capture the fork event and record the app PID.
            if (ptrace(PTRACE_GETEVENTMSG, pid, NULL, &pid_app_) == -1)
                LOG_SYSERR_AND_THROW()

            if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1)
                LOG_SYSERR_AND_THROW()
            break;
        }
    }
}

void EggHunter::CheckStartupCmd(const char* sz_app)
{
    // Wait for the new process to finish app initialization.
    while (true) {
        int32_t status;
        if (waitpid(pid_app_, &status, __WALL) != pid_app_)
            LOG_SYSERR_AND_THROW()

        // Force the newly forked process to stop at each system call entry or
        // the code points just after system call. So that we can capture the
        // timing about finishing initialization.
        if (ptrace(PTRACE_SYSCALL, pid_app_, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        char buf[SIZE_MID_BLAH];
        sprintf(buf, "/proc/%d/cmdline", pid_app_);

        std::ifstream cmd(buf, std::ifstream::in);
        if (cmd.good())
            cmd.getline(buf, SIZE_MID_BLAH);
        if (strstr(buf, sz_app)) {
            LOG("[+] Capture the target app %d -> %s\n", pid_app_, buf);
            break;
        }
    }
}

int32_t EggHunter::CaptureApp(pid_t pid_zygote, const char* sz_app)
{
    int32_t rtn = SUCC;
    try {
        /*
         * First, we need to attach to the zygote process and wait for the
         * target app to be forked after our triggering.
         */
        if (ptrace(PTRACE_ATTACH, pid_zygote, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        int32_t status;
        if (waitpid(pid_zygote, &status, WUNTRACED) != pid_zygote)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETOPTIONS, pid_zygote, 1, PTRACE_O_TRACEFORK) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_zygote, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        /*
         * Second, we enter the polling loop to wait for the target app. When
         * we catch the fork event fired by zygote, we should verify if the
         * newly forked app is our target by examining the startup command. If
         * so, we successfully get the goal and should release zygote.
         */
        WaitForForkEvent(pid_zygote);
        CheckStartupCmd(sz_app);
    } catch (BadProbe& e) {
        rtn = FAIL;
        if (pid_app_ != 0)
            ptrace(PTRACE_DETACH, pid_app_, NULL, NULL);
    }

    ptrace(PTRACE_DETACH, pid_zygote, NULL, NULL);
    return rtn;
}

int32_t EggHunter::InjectApp(const char* sz_path)
{
    pid_t pid_inject = getpid();

    if (func_tbl_.Resolve(pid_inject, pid_app_) != SUCC)
        return FAIL;

    return SUCC;
}

int32_t EggHunter::Hunt(pid_t pid_zygote, const char* sz_app, const char* sz_path)
{
    if (CaptureApp(pid_zygote, sz_app) != SUCC)
        return FAIL;

    if (InjectApp(sz_path) != SUCC)
        return FAIL;

    return SUCC;
}

}