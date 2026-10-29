#include "boundary.h"

int ld_bit_encode(int32_t value, unsigned char out[2])
{
    if (!out || value < -4096 || value > 4095)
    {
        return 0;
    }
    _BitInt(13) s = (_BitInt(13))value;
    unsigned _BitInt(13) u = (unsigned _BitInt(13))s;
    unsigned int wire = (unsigned int)u;
    out[0] = (unsigned char)(wire >> 8);
    out[1] = (unsigned char)(wire & 255u);
    return 1;
}

int ld_bit_decode(const unsigned char in[2], int32_t *out)
{
    if (!in || !out || in[0] > 31)
    {
        return 0;
    }
    unsigned _BitInt(13) u =
        (unsigned _BitInt(13))((unsigned)in[0] * 256u + in[1]);
    /* Arithmetic sign decoding avoids implementation-defined out-of-range
       unsigned-to-signed conversion even in this C23 implementation. */
    unsigned int wire = (unsigned int)u;
    *out = wire < 4096u ? (int32_t)wire : (int32_t)wire - 8192;
    return 1;
}
