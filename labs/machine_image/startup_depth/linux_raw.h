#ifndef CM_STARTUP_LINUX_RAW_H
#define CM_STARTUP_LINUX_RAW_H
#include "startup.h"
/* Linux x86-64 LP64 only. The number and arguments are machine words. This
 * interface is not an ISO C system-call facility. Only original local process
 * descriptors/mappings are permitted by this laboratory. No global errno. */
uint64_t sd_raw6(uint64_t number, uint64_t a, uint64_t b, uint64_t c,
                 uint64_t d, uint64_t e, uint64_t f);

struct sd_fd
{
    unsigned fd;
    int reading;
};
struct sd_result sd_fd_backend(void *, unsigned char *, size_t);
/* Assembly entry captures these before any C prologue; no ordinary caller
 * return address exists at process entry. This instrumentation is a supplied
 * target contract, not a portable way to introspect a C stack. */
extern uintptr_t sd_original_sp, sd_call_sp, sd_helper_sp;
#endif
