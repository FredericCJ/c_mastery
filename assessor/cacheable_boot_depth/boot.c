#include "boot.h"
#ifndef CB_MUTANT
#define CB_MUTANT 0
#endif
#define RCC UINT32_C(0x58024400)
#define CCR UINT32_C(0xE000ED14)
#define TYPE UINT32_C(0xE000ED90)
#define CTRL UINT32_C(0xE000ED94)
#define RNR UINT32_C(0xE000ED98)
#define RBAR UINT32_C(0xE000ED9C)
#define RASR UINT32_C(0xE000EDA0)
#define CSSELR UINT32_C(0xE000ED84)
#define CCSIDR UINT32_C(0xE000ED80)
#define DCISW UINT32_C(0xE000EF60)

static int valid(struct cb_io x)
{
    return x.registers.read && x.registers.write && x.registers.sync && x.isb;
}

static uint32_t rd(struct cb_io x, uint32_t a)
{
    return x.registers.read(x.registers.ctx, a);
}

static void wr(struct cb_io x, uint32_t a, uint32_t v)
{
    x.registers.write(x.registers.ctx, a, v);
}

static void dsb(struct cb_io x)
{
    x.registers.sync(x.registers.ctx);
}

static void isb(struct cb_io x)
{
    x.isb(x.registers.ctx);
}

static int regions(struct cb_io x)
{
    uint32_t t = rd(x, TYPE);
    unsigned n = (unsigned)((t >> 8) & 255u);
    return (t & 1u) || (CB_MUTANT == 6 ? (n < 8u || n > 16u)
                                       : (n != 8u && n != 16u))
               ? 0
               : (int)n;
}

static void map(struct cb_io x, unsigned n, uint32_t attr)
{
    dsb(x);
    for (unsigned i = 0; i < n; ++i)
    {
        wr(x, RNR, i);
        wr(x, RASR, 0);
    }
    wr(x, RNR, 7);
    wr(x, RBAR, CB_LINE);
    wr(x, RASR, attr);
    wr(x, CTRL, 5);
    dsb(x);
    isb(x);
}

static void emit(void *v, enum fw_cache_op op, uint32_t a)
{
    struct cb_io *x = v;
    if (op == FW_DSB)
    {
        dsb(*x);
    }
    else if (op == FW_ISB)
    {
        isb(*x);
    }
    else
    {
        wr(*x, op == FW_CLEAN ? UINT32_C(0xE000EF68) : UINT32_C(0xE000EF5C), a);
    }
}

int cb_boot(struct cb_io x, uint32_t g)
{
    if (!valid(x) || !g)
    {
        return 0;
    }
    int n = regions(x);
    if (!n || (rd(x, CCR) & UINT32_C(0x30000)) || rd(x, CTRL) ||
        (rd(x, RCC + 0xA0u) & 8u))
    {
        return 0;
    }
    /* This writes only the selector before unsupported geometry refusal. */
    wr(x, CSSELR, 0);
    dsb(x);
    uint32_t geometry = rd(x, CCSIDR);
    unsigned sets = (unsigned)((geometry >> 13) & 32767u) + 1u;
    if ((geometry & 8191u) != 25u || (geometry >> 28) != 15u ||
        (sets != 32u && sets != 64u && sets != 128u && sets != 256u &&
         sets != 512u))
    {
        return 0;
    }
    if (!st_cold_h745(x.registers))
    {
        return 0;
    }
    if (rd(x, UINT32_C(0x58026400)) & UINT32_C(0x80000000))
    {
        return 0;
    }
    map(x, (unsigned)n, CB_MUTANT == 2 ? CB_M4_RASR : CB_M7_RASR);
    /* Selected four-way,32-byte geometry. No undefined shift or CLZ(0). */
    if (CB_MUTANT != 3)
    {
        dsb(x);
        for (unsigned s = 0; s < sets; ++s)
        {
            for (unsigned w = 0; w < 4u; ++w)
            {
                wr(x, DCISW, ((uint32_t)w << 30) | ((uint32_t)s << 5));
            }
        }
        dsb(x);
        isb(x);
    }
    wr(x, CCR, rd(x, CCR) | UINT32_C(0x10000));
    dsb(x);
    isb(x);
    if (CB_MUTANT == 1)
    {
        wr(x, RCC + 0xA0u, rd(x, RCC + 0xA0u) | 8u);
    }
    for (unsigned i = 0; i < 8u; ++i)
    {
        wr(x, CB_LINE + 4u * i, i ? 0 : g);
    }
    if (CB_MUTANT != 4)
    {
        (void)fw_cache_range(CB_LINE, 32, FW_CLEAN, emit, &x);
    }
    if (CB_MUTANT != 1)
    {
        wr(x, RCC + 0xA0u, rd(x, RCC + 0xA0u) | 8u);
    }
    dsb(x);
    return 1;
}

int cb_peer(struct cb_io x, uint32_t g)
{
    if (!valid(x) || !g)
    {
        return 0;
    }
    int n = regions(x);
    if (!n || rd(x, CTRL) || (rd(x, CCR) & UINT32_C(0x30000)) ||
        !(rd(x, RCC + 0xA0u) & 8u) ||
        (rd(x, RCC + 0xDCu) & UINT32_C(0xA0000000)) != UINT32_C(0xA0000000) ||
        !(rd(x, RCC + 0xE0u) & (UINT32_C(1) << 25)))
    {
        return 0;
    }
    map(x, (unsigned)n, CB_M4_RASR);
    dsb(x);
    if (CB_MUTANT == 5)
    {
        wr(x, CB_LINE, g);
    }
    return rd(x, CB_LINE) == g;
}
