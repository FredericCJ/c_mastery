#include "declarations.h"
#ifndef LD_MUTANT
#define LD_MUTANT 0
#endif
enum ld_status ld_encode(int32_t v, unsigned char out[2])
{
    if (!out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT == 4
    out[0] = 0;
#endif
    if (v < -4096 || v > 4095)
    {
        return LD_INVALID;
    }
    uint32_t u = v < 0 ? (uint32_t)(v + 8192) : (uint32_t)v;
    out[0] = (unsigned char)(u >> 8);
    out[1] = (unsigned char)(u & 255u);
    return LD_OK;
}

enum ld_status ld_decode(const unsigned char in[2], int32_t *out)
{
    if (!in || !out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT != 1
    if (in[0] > 31)
    {
        return LD_INVALID;
    }
#endif
    uint32_t u = (uint32_t)(in[0] & 31u) * 256u + in[1];
#if LD_MUTANT == 2
    *out = (int32_t)u;
#else
    *out = u < 4096u ? (int32_t)u : (int32_t)u - 8192;
#endif
    return LD_OK;
}

enum ld_status ld_triangle(uint32_t n, uint32_t *out)
{
    if (!out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT == 3
    *out = 0;
#endif
    if (n > 65535u)
    {
        return LD_INVALID;
    }
    uint32_t a = n, b = n + 1u;
    if (a % 2u == 0)
    {
        a /= 2u;
    }
    else
    {
        b /= 2u;
    }
    *out = a * b;
    return LD_OK;
}
