#include "hardening.h"

int sh_copy(const unsigned char *src, size_t count, unsigned *out)
{
    (void)src;
    (void)count;
    (void)out;
    return -1;
}

int sh_authorize(unsigned claimed_role, int trusted, unsigned *counter)
{
    (void)claimed_role;
    (void)trusted;
    (void)counter;
    return -1;
}
