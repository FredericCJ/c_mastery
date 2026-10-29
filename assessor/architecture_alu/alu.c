#include "alu.h"

enum aa_status aa_slice(unsigned a, unsigned b, unsigned in,
                        struct aa_slice *out)
{
    if (!out || a > 1 || b > 1 || in > 1)
        return AA_INVALID;
    struct aa_slice r = {a ^ b ^ in, (a & b) | (a & in) | (b & in)};
    *out = r;
    return AA_OK;
}

enum aa_status aa_alu(unsigned width, unsigned op, unsigned a, unsigned b,
                      unsigned input, struct aa_result *out)
{
    if (!out || (width != 4 && width != 5) || input > 1)
        return AA_INVALID;
#ifndef AA_MUTANT_ADMISSION
    if (op > 1)
        return AA_INVALID;
#else
    op &= 1;
#endif
    unsigned mask = (1u << width) - 1u;
    if (a > mask || b > mask)
        return AA_INVALID;
    struct aa_result r = {0};
    r.carry[0] = op ? 1u - input : input;
#ifdef AA_MUTANT_BORROW
    if (op)
        r.carry[0] = 1;
#endif
    for (unsigned i = 0; i < width; ++i)
    {
        struct aa_slice slice;
        if (aa_slice((a >> i) & 1u, ((b >> i) & 1u) ^ op, r.carry[i], &slice) !=
            AA_OK)
            return AA_INVALID;
        r.bit[i] = slice.sum;
        r.carry[i + 1] = slice.carry;
        r.value |= slice.sum << i;
    }
    r.z = r.value == 0;
    r.n = (r.value >> (width - 1)) & 1u;
    r.c_arm = r.carry[width];
    r.cf_x86 = op ? 1u - r.c_arm : r.c_arm;
    r.v = r.carry[width - 1] ^ r.carry[width];
#ifdef AA_MUTANT_WIRES
    unsigned temp = r.v;
    r.v = r.c_arm;
    r.c_arm = temp;
#endif
    *out = r;
    return AA_OK;
}

unsigned aa_relations(const struct aa_result *r)
{
    unsigned less = r->n ^ r->v;
#ifdef AA_MUTANT_SIGNED
    less = r->n;
#endif
    return (1u - r->c_arm) | ((r->c_arm && !r->z) << 1) | (less << 2) |
           ((!less && !r->z) << 3) | (r->z << 4);
}
