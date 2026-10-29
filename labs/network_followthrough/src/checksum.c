/* Import your personally completed X-T41-001 implementation here. */
#include "systems.h"

int s_checksum_add(struct s_checksum *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return S_INVALID;
}

int s_checksum_finish(const struct s_checksum *s, uint16_t *out)
{
    (void)s;
    (void)out;
    return S_INVALID;
}
