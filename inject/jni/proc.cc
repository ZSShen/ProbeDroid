#include <proc.h>


namespace proc {

using namespace util;

#define PATH_LINKER             "/system/bin/linker"
#define PATH_LIBC               "/system/lib/libc.so"
#define NAME_LINKER             "libdl.so"
#define FUNC_DLOPEN             "dlopen"
#define FUNC_MMAP               "mmap"
#define PERM_READ_EXEC          "r-xp"
#define SIZE_SEGMENT            0x1000

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

            int32_t status;
            if (waitpid(pid_app_, &status, __WALL) != pid_app_)
                LOG_SYSERR_AND_THROW()
            break;
        }
    }
}

void EggHunter::CheckStartupCmd(const char* sz_app)
{
    // Wait for the new process to finish app initialization.
    while (true) {
        // Force the newly forked process to stop at each system call entry or
        // the code points just after system call. So that we can capture the
        // timing about finishing initialization.
        if (ptrace(PTRACE_SYSCALL, pid_app_, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        int32_t status;
        if (waitpid(pid_app_, &status, __WALL) != pid_app_)
            LOG_SYSERR_AND_THROW()

        char buf[SIZE_MID_BLAH];
        sprintf(buf, "/proc/%d/cmdline", pid_app_);

        std::ifstream cmd(buf, std::ifstream::in);
        if (cmd.good())
            cmd.getline(buf, SIZE_MID_BLAH);
        if (strstr(buf, sz_app)) {
            LOG("[+] Capture the target app %s(%d)", buf, pid_app_);
            break;
        }
    }
}

int32_t EggHunter::PokeTextInApp(uint32_t addr_txt, const char* buf, int32_t count_byte)
{
    int32_t count_wrt = 0;
    while (count_wrt < count_byte) {
        int32_t word;
        memcpy(&word, buf + count_wrt, sizeof(word));
        if (ptrace(PTRACE_POKETEXT, pid_app_, addr_txt + count_wrt, word) == -1)
            return FAIL;
        count_wrt += sizeof(word);
    }
    return SUCC;
}

int32_t EggHunter::PeekTextInApp(uint32_t addr_txt, char* buf, int32_t count_byte)
{
    // Expect type coercion. But better approach is needed.
    int32_t* slide = (int32_t*)buf;
    int32_t count_read = 0, idx = -1;
    while (count_read < count_byte) {
        int32_t word = ptrace(PTRACE_PEEKTEXT, pid_app_, addr_txt + count_read, NULL);
        if (word == -1)
            return FAIL;
        count_read += sizeof(word);
        slide[++idx] = word;
    }
    return SUCC;
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

    // Prepare the library pathname. zero padding is necessary to produce
    // the text word applied by ptrace().
    char payload[SIZE_MID_BLAH];
    int32_t len_payload = strlen(sz_path);
    strncpy(payload, sz_path, len_payload);
    payload[len_payload++] = 0;

    div_t count_word = div(len_payload, sizeof(uint32_t));
    int32_t patch = (count_word.rem > 0)? (sizeof(uint32_t) - count_word.rem) : 0;
    for (int i = 0 ; i < patch ; ++i)
        payload[len_payload++] = 0;

    // Prepare the addresses of mmap() and dlopen() of the target app.
    uint32_t addr_mmap = func_tbl_.GetMmap();
    uint32_t addr_dlopen = func_tbl_.GetDlopen();

    int32_t rtn = SUCC;
    try {
        // Backup the context of the target app.
        struct user_regs_struct reg_orig;
        if (ptrace(PTRACE_GETREGS, pid_app_, NULL, &reg_orig) == -1)
            LOG_SYSERR_AND_THROW();

        /*
         * First, we force the target app to execute mmap(). The allocated
         * block will be stuffed with the pathname of the hooking library.
         * Note, to fit the calling convention,
         * param[0] stores the return address, and
         * param[6] to param[1] is the actual parameters of mmap().
         */
        int32_t param[SIZE_TINY_BLAH];
        param[0] = 0;
        param[1] = 0;
        param[2] = SIZE_SEGMENT;
        param[3] = PROT_READ | PROT_WRITE | PROT_EXEC;
        param[4] = MAP_ANONYMOUS | MAP_PRIVATE;
        param[5] = 0;
        param[6] = 0;
        int32_t count_byte = sizeof(uint32_t) * 7;

        struct user_regs_struct reg_modi;
        memcpy(&reg_modi, &reg_orig, sizeof(struct user_regs_struct));
        reg_modi.eip = addr_mmap;
        reg_modi.esp -= count_byte;

        // Expect type coercion. But better approach is needed.
        if (PokeTextInApp(reg_modi.esp, (char*)param, count_byte) != SUCC)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETREGS, pid_app_, NULL, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        // The injector will receive a SIGSEGV triggered by target app due to
        // invalid return address.
        int32_t status;
        if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
            LOG_SYSERR_AND_THROW()

        // Stuff the pathname of the hooking library into the newly mapped
        // memory block.
        if (ptrace(PTRACE_GETREGS, pid_app_, NULL, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        uint32_t addr_blk = reg_modi.eax;
        LOG("[+] mmap() successes with %x returned", addr_blk);

        if (PokeTextInApp(addr_blk, payload, len_payload) == -1)
            LOG_SYSERR_AND_THROW()

        /*
         * Second, we force the target app to execute dlopen(). Then our hooking
         * library will be loaded by target.
         * Note, to fit the calling convention,
         * param[0] stores the return address, and
         * param[2] to param[1] is the actual parameters of dlopen().
         */
        param[0] = 0;
        param[1] = addr_blk;
        param[2] = RTLD_NOW;
        count_byte = sizeof(uint32_t) * 3;

        reg_modi.eip = addr_dlopen;
        reg_modi.esp -= count_byte;

        // Expect type coercion. But better approach is needed.
        if (PokeTextInApp(reg_modi.esp, (char*)param, count_byte) != SUCC)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETREGS, pid_app_, NULL, &reg_modi) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        // The injector will receive a SIGSEGV triggered by target app due to
        // invalid return address.
        if (waitpid(pid_app_, &status, WUNTRACED) != pid_app_)
            LOG_SYSERR_AND_THROW()
        LOG("[+] dlopen() successes");

        // At this stage, we finish the library injection and should restore
        // the context of the target app.
        if (ptrace(PTRACE_SETREGS, pid_app_, NULL, &reg_orig) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_app_, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()
    } catch (BadProbe& e) {
        rtn = FAIL;
        if (pid_app_ != 0)
            ptrace(PTRACE_DETACH, pid_app_, NULL, NULL);
    }

    ptrace(PTRACE_DETACH, pid_app_, NULL, NULL);
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