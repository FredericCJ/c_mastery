#define _GNU_SOURCE
#include <dlfcn.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#ifndef AE_BUILD
#define AE_BUILD 4
#endif
__attribute__((noinline)) unsigned ae_fault_site(unsigned value)
{
    volatile unsigned revision = AE_BUILD;
    return value + revision;
}

int main(void)
{
    Dl_info info;
    uintptr_t address = (uintptr_t)&ae_fault_site;
    if (!dladdr((void *)address, &info))
    {
        return 1;
    }
    printf("BUILD=%u ADDRESS=%" PRIxPTR " BIAS=%" PRIxPTR " VALUE=%u\n",
           AE_BUILD, address, (uintptr_t)info.dli_fbase, ae_fault_site(1));
    return 0;
}
