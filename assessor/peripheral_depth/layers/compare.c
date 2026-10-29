#include "compare.h"
#include "registers.h"
#ifndef PL_COMPARE_MUTANT
#define PL_COMPARE_MUTANT 0
#endif
static int wait_flag(struct pl_byte_port p, uint32_t flag, uint32_t start,
                     uint32_t timeout)
{
    while (!(*p.status & flag))
    {
        uint32_t now = p.tick(p.context);
        if ((uint32_t)(now - start) > timeout || !timeout)
        {
            return PL_TIMEOUT;
        }
    }
    return PL_OK;
}

int pl_byte(struct pl_byte_port p, uint8_t b, uint32_t timeout)
{
    if (!p.status || !p.data || !p.tick || !p.txe || !p.tc ||
        timeout >= 0x80000000u)
    {
        return PL_INVALID;
    }
    uint32_t start = p.tick(p.context);
    int r = wait_flag(p, p.txe, start, timeout);
    if (r)
    {
        return r;
    }
    *p.data = b;
    return PL_COMPARE_MUTANT == 1 ? PL_OK : wait_flag(p, p.tc, start, timeout);
}

struct pl_byte_port pl_direct_f439(volatile uint32_t *p, pl_tick_fn t, void *c)
{
    if (!p)
    {
        return (struct pl_byte_port){0};
    }
    return (struct pl_byte_port){p, p + 1, 128, 64, t, c};
}

struct pl_byte_port pl_direct_g474(volatile uint32_t *p, pl_tick_fn t, void *c)
{
    if (!p)
    {
        return (struct pl_byte_port){0};
    }
    return (struct pl_byte_port){p + 7, p + 10, 128, 64, t, c};
}

struct pl_byte_port pl_generated(volatile uint32_t *p, unsigned revision,
                                 pl_tick_fn t, void *c)
{
    if (!p || revision > 1)
    {
        return (struct pl_byte_port){0};
    }
    if (revision && PL_COMPARE_MUTANT != 2)
    {
        return (struct pl_byte_port){p + PL_G474_STATUS / 4,
                                     p + PL_G474_DATA / 4,
                                     PL_G474_TXE,
                                     PL_G474_TC,
                                     t,
                                     c};
    }
    return (struct pl_byte_port){p + PL_F439_STATUS / 4,
                                 p + PL_F439_DATA / 4,
                                 PL_F439_TXE,
                                 PL_F439_TC,
                                 t,
                                 c};
}
