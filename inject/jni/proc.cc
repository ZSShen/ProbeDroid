#include <proc.h>

namespace proc {

using namespace util;


#define LOG_SYSERR_AND_THROW()                                                 \
        do {                                                                   \
            ERROR("[-] %s", strerror(errno));                                  \
            throw bad_probe();                                                 \
        } while (0);


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

int32_t EggHunter::CaptureApp(pid_t pid_zygote)
{
    int32_t rtn = SUCC;

    try {
        /*---------------------------------------------------------------------*
         * First, we need to attach to the zygote process and wait for the     *
         * target app to be forked after our triggering.                       *
         *---------------------------------------------------------------------*/
        if (ptrace(PTRACE_ATTACH, pid_zygote, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        int32_t status;
        if (waitpid(pid_zygote, &status, WUNTRACED) != pid_zygote)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_SETOPTIONS, pid_zygote, 1, PTRACE_O_TRACEFORK) == -1)
            LOG_SYSERR_AND_THROW()

        if (ptrace(PTRACE_CONT, pid_zygote, NULL, NULL) == -1)
            LOG_SYSERR_AND_THROW()

        /*---------------------------------------------------------------------*
         * Second, we enter the polling loop to wait for the target app. When  *
         * we catch the fork event fired by zygote, we should verify if the    *
         * newly forked app is our target by examining the startup command. If *
         * so, we successfully get the goal and should release zygote.         *
         *---------------------------------------------------------------------*/
        while (true) {
            WaitForForkEvent(pid_zygote);
            LOG("[+] Child PID: %d\n", pid_app_);
        }
    } catch (bad_probe& e) {
        rtn = FAIL;
        if (pid_app_ != 0)
            ptrace(PTRACE_DETACH, pid_app_, NULL, NULL);
    }

    ptrace(PTRACE_DETACH, pid_zygote, NULL, NULL);
    return rtn;
}

int32_t EggHunter::Hunt(pid_t pid_zygote)
{
    if (CaptureApp(pid_zygote) != SUCC)
        return FAIL;

    return SUCC;
}

}