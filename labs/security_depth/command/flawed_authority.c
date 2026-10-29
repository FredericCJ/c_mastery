/* Intentionally vulnerable ORIGINAL target. Only opcode2/count0 and a live
 * synthetic counter are admitted. This demonstrates an unauthorized integer
 * effect, never arbitrary native memory access, code execution or host access.
 */
#include "command.h"

enum sc_result sc_flawed_authority(const unsigned char *p, size_t n,
                                   uint32_t *c)
{
    if (!p || !c || n != 2 || p[0] != 2 || p[1] != 0)
    {
        return SC_INVALID;
    }
    if (*c == UINT32_MAX)
    {
        return SC_EXHAUSTED;
    }
    ++*c;
    return SC_OK;
}
