#include "declarations.h"
#ifndef LD_MUTANT
#define LD_MUTANT 0
#endif
/* The wire value is a 13-bit two's-complement integer in a big-endian pair:
   the high byte carries bits 12..8, so it never exceeds 31. */
enum
{
    VALUE_MIN = -4096,
    VALUE_MAX = 4095,
    VALUE_MODULUS = 8192,
    HIGH_BYTE_MASK = 31,
    TRIANGLE_MAX_INPUT = 65535
};

/* Encode v in [-4096, 4095] into out[0..1]. Returns LD_INVALID for a null out
   or an out-of-range v, leaving out unchanged. */
enum ld_status ld_encode(int32_t v, unsigned char out[2])
{
    if (!out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT == 4
    out[0] = 0;
#endif
    if (v < VALUE_MIN || v > VALUE_MAX)
    {
        return LD_INVALID;
    }
    uint32_t u = v < 0 ? (uint32_t)(v + VALUE_MODULUS) : (uint32_t)v;
    out[0] = (unsigned char)(u >> 8);
    out[1] = (unsigned char)(u & 255u);
    return LD_OK;
}

/* Decode a canonical pair. Non-canonical input (any of the top three bits of
   in[0] set) is refused with LD_INVALID, leaving *out unchanged. */
enum ld_status ld_decode(const unsigned char in[2], int32_t *out)
{
    if (!in || !out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT != 1
    if (in[0] > HIGH_BYTE_MASK)
    {
        return LD_INVALID;
    }
#endif
    uint32_t u = (uint32_t)(in[0] & HIGH_BYTE_MASK) * 256u + in[1];
#if LD_MUTANT == 2
    *out = (int32_t)u;
#else
    *out = u < 4096u ? (int32_t)u : (int32_t)u - VALUE_MODULUS;
#endif
    return LD_OK;
}

/* n * (n + 1) / 2 for n <= 65535 (the result fits in 32 bits). One of n and
   n + 1 is even; halving it before the multiply keeps the division exact and
   every intermediate in 32 bits. Returns LD_INVALID for a null out or a
   larger n, leaving *out unchanged. */
enum ld_status ld_triangle(uint32_t n, uint32_t *out)
{
    if (!out)
    {
        return LD_INVALID;
    }
#if LD_MUTANT == 3
    *out = 0;
#endif
    if (n > TRIANGLE_MAX_INPUT)
    {
        return LD_INVALID;
    }
    uint32_t a = n;
    uint32_t b = n + 1u;
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
