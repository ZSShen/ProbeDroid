
#include "gadget.h"
#include "logcat.h"
#include <stdint.h>
#include <mutex>


thread_local uintptr_t tls_eip;
thread_local uintptr_t tls_esi;


void StashReturnAddress(uintptr_t eip)
{
    tls_eip = eip;
}

void StashESI(uintptr_t esi)
{
    tls_esi = esi;
}

uintptr_t RestoreReturnAddress()
{
    return tls_eip;
}

uintptr_t RestoreESI()
{
    return tls_esi;
}
