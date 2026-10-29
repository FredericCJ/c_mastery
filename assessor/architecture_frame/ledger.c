#include "ledger.h"

int mf_reset(struct mf_boot c, const uint32_t *table, size_t n,
             struct mf_reset *out)
{
    if (!table || !out || c.boot > MF_SRAM || c.irq > 239 ||
        c.stack_low >= c.stack_high || c.code_low >= c.code_high)
    {
        return MF_INVALID;
    }
    uint32_t base = c.boot == MF_FLASH ? 0x08000000u : 0x20000000u;
    if (c.vtor != 0 && c.vtor != base)
    {
        return MF_UNSUPPORTED;
    }
    uint32_t extent = c.boot == MF_FLASH ? 0x200000u : 0x1c000u;
    if (c.code_low < base || c.code_high - base > extent)
    {
        return MF_INVALID;
    }
    if (n < 2 || n <= 16u + c.irq)
    {
        return MF_SHORT;
    }
    uint32_t msp = table[0], vector = table[1];
    if (msp <= c.stack_low || msp > c.stack_high || msp % 8 || !(vector & 1u))
    {
        return MF_STATE;
    }
    uint32_t pc = vector & ~1u;
    if (pc < c.code_low || pc >= c.code_high)
    {
        return MF_STATE;
    }
    struct mf_reset r = {{0, 4}, {base, base + 4}, msp,
                         pc,     16u + c.irq,      c.vtor + 4u * (16u + c.irq)};
#ifdef MF_MUTANT_RESET
    r.reset_fetch[0] = c.vtor;
    r.reset_fetch[1] = c.vtor + 4;
#endif
#ifdef MF_MUTANT_IRQ
    r.irq_vector = c.vtor + 4u * c.irq;
#endif
    *out = r;
    return MF_OK;
}

int mf_capture(const uint32_t *w, size_t n, uint32_t sp, uint32_t er,
               unsigned policy, unsigned fpu, unsigned material, unsigned align,
               unsigned fault, struct xf_frame *out)
{
    if (!out || !w || policy > 1 || fpu > 1 || material > 1 || align > 1 ||
        fault > 1)
    {
        return MF_INVALID;
    }
    if (fault)
    {
        return MF_STATE;
    }
    if (!policy)
    {
        if (er != 0xfffffffdu)
        {
            return MF_UNSUPPORTED;
        }
        if (n < 8)
        {
            return MF_SHORT;
        }
#ifndef MF_MUTANT_POLICY
        if (w[7] & (1u << 9))
        {
            return MF_UNSUPPORTED;
        }
#endif
    }
    struct xf_frame r;
    int rc = xf_frame(w, n, sp, er, fpu, material, align, 0, &r);
    if (rc != XF_OK)
    {
        return rc == XF_SHORT         ? MF_SHORT
               : rc == XF_UNSUPPORTED ? MF_UNSUPPORTED
                                      : MF_STATE;
    }
#ifdef MF_MUTANT_LR
    r.stacked_lr = er;
#endif
    *out = r;
    return MF_OK;
}

int mf_ownership(unsigned r9, struct mf_ownership *out)
{
    if (r9 > 1 || !out)
    {
        return MF_INVALID;
    }
    struct mf_ownership r = {0x1d00fu, 0x2df0u | (r9 ? 0x200u : 0u), 0x1500fu,
                             !r9};
#ifdef MF_MUTANT_R4
    r.hardware |= 1u << 4;
#endif
    *out = r;
    return MF_OK;
}

int mf_nested(const struct xf_frame *a, uint32_t as, unsigned exception,
              const struct xf_frame *b, uint32_t bs,
              struct xf_priority_config cfg, unsigned ap, unsigned bp,
              struct mf_return_order *out)
{
    if (!a || !b || !out || exception < 16 || exception > 511)
    {
        return MF_INVALID;
    }
    if (!a->thread || !a->psp || b->thread || b->psp ||
        (b->xpsr & 511u) != exception || a->original_sp <= as ||
        b->original_sp <= bs || !(a->original_sp <= bs || b->original_sp <= as))
    {
        return MF_STATE;
    }
    struct xf_priority pa, pb;
    if (xf_priority(cfg, ap, &pa) || xf_priority(cfg, bp, &pb))
    {
        return MF_INVALID;
    }
    if (pb.group >= pa.group)
    {
        return MF_STATE;
    }
    struct mf_return_order r = {b->pc, b->original_sp, a->pc, a->original_sp, 0,
                                1};
#ifdef MF_MUTANT_STACK
    r.first_stack = 1;
#endif
    *out = r;
    return MF_OK;
}
