#include "power.h"
#include <limits.h>
#ifndef CS_POWER_MUTANT
#define CS_POWER_MUTANT 0
#endif
static int bus(const struct cs_bus *b)
{
    return b && b->read && b->write && b->idle;
}

enum cs_status cs_stop_init(struct cs_stop *s)
{
    if (!s)
    {
        return CS_INVALID;
    }
    *s = (struct cs_stop){0};
    return CS_OK;
}

enum cs_status cs_stop_enter(struct cs_stop *s, const struct cs_bus *b,
                             unsigned premises)
{
    if (!s || !bus(b) || premises != 31 || s->phase)
    {
        return CS_INVALID;
    }
    if (s->epoch == UINT_MAX)
    {
        return CS_EXHAUSTED;
    }
    uint32_t cr = b->read(b->context, CS_RCC_CR),
             cfgr = b->read(b->context, CS_RCC_CFGR);
    uint32_t pwr = b->read(b->context, CS_PWR_CR),
             scr = b->read(b->context, CS_SCR);
    if ((cr & 3) != 3 || (cr & UINT32_C(0x15090000)) ||
        (cfgr & UINT32_C(0xfcff)) ||
        !(b->read(b->context, CS_RCC_APB1ENR) & (1u << 28)) ||
        (pwr & UINT32_C(0xf0c00)) || (scr & 6) ||
        (b->read(b->context, CS_DBG) & 7) ||
        (b->read(b->context, CS_SYSTICK) & 3))
    {
        return CS_INVALID;
    }
    if (CS_POWER_MUTANT != 1 && (b->read(b->context, CS_IWDG_SR) & 3))
    {
        return CS_BUSY;
    }
    b->write(b->context, CS_PWR_CR, pwr & ~UINT32_C(0xe03));
    b->write(b->context, CS_SCR, scr | 4);
    b->idle(b->context);
    /* No callback except the owned wake instruction can escape this sequence. */
    b->write(b->context, CS_SCR, scr);
    b->write(b->context, CS_PWR_CR, pwr);
    s->phase = 1;
    return CS_OK;
}

enum cs_status cs_stop_finish(struct cs_stop *s, const struct cs_bus *b,
                              unsigned known, uint64_t before, uint64_t after)
{
    if (!s || !bus(b) || s->phase != 1 || known > 1)
    {
        return CS_INVALID;
    }
    if (known && after < before)
    {
        return CS_INVALID;
    }
    if (CS_POWER_MUTANT != 2 && ((b->read(b->context, CS_RCC_CR) & 3) != 3 ||
                                 (b->read(b->context, CS_RCC_CFGR) & 15)))
    {
        return CS_WAIT;
    }
    if (s->epoch == UINT_MAX)
    {
        return CS_EXHAUSTED;
    }
    ++s->epoch;
    s->phase = 2;
    s->elapsed_known = known;
    s->elapsed = known ? after - before : 0;
    return CS_OK;
}

enum cs_status cs_iwdg_start(const struct cs_bus *b, unsigned prescaler,
                             unsigned reload, unsigned budget)
{
    if (!bus(b) || prescaler > 6 || reload > 4095 || !budget)
    {
        return CS_INVALID;
    }
    b->write(b->context, CS_IWDG_KR, 0xcccc);
    b->write(b->context, CS_IWDG_KR, 0x5555);
    b->write(b->context, CS_IWDG_PR, prescaler);
    b->write(b->context, CS_IWDG_RLR, reload);
    for (unsigned i = 0; i < budget; ++i)
    {
        if (!(b->read(b->context, CS_IWDG_SR) & 3))
        {
            b->write(b->context, CS_IWDG_KR, 0xaaaa);
            return CS_OK;
        }
    }
    return CS_WAIT;
}

enum cs_status cs_iwdg_feed(struct cs_world *w, const struct cs_bus *b)
{
    if (!bus(b))
    {
        return CS_INVALID;
    }
    enum cs_status result = cs_window(w);
    if (result == CS_OK || CS_POWER_MUTANT == 3)
    {
        b->write(b->context, CS_IWDG_KR, 0xaaaa);
    }
    return result;
}
