#include <stddef.h>
#include <stdint.h>
#ifndef DEBUG_REPAIRED
#define DEBUG_REPAIRED 0
#endif
/* Every read remains inside the live input; defect is an endian semantic error. */
uint32_t debug_decode(const unsigned char *p, size_t n, unsigned live_owner)
{
    if (!live_owner || n != 2)
    {
        return UINT32_MAX;
    }
    uint32_t value = DEBUG_REPAIRED ? ((uint32_t)p[0] << 8) | p[1]
                                    : ((uint32_t)p[1] << 8) | p[0];
    return value;
}
