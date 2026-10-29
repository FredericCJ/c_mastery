#include "declarations.h"

enum ld_status ld_encode(int32_t v, unsigned char out[2])
{
    (void)v;
    (void)out;
    return LD_INVALID;
}

enum ld_status ld_decode(const unsigned char in[2], int32_t *out)
{
    (void)in;
    (void)out;
    return LD_INVALID;
}

enum ld_status ld_triangle(uint32_t n, uint32_t *out)
{
    (void)n;
    (void)out;
    return LD_INVALID;
}
