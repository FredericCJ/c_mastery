#include "timer.h"
#ifndef PT_TARGET_MUTANT
#define PT_TARGET_MUTANT 0
#endif
#ifdef PT_TARGET_MODEL
uint32_t pt_read(uintptr_t);
void pt_write(uintptr_t, uint32_t);
#else
_Static_assert(sizeof(uintptr_t) == 4, "F439 address width");
#define pt_read(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define pt_write(a, v) (*(volatile uint32_t *)(uintptr_t)(a) = (uint32_t)(v))
#endif
#define T3 0x40000400u
#define T4 0x40000800u
#define APB 0x40023840u
#define RESET 0x40023820u
static uint32_t generation, configured_clock;
static unsigned mode;

static void gate(uint32_t bits)
{
    pt_write(APB, pt_read(APB) | bits);
    (void)pt_read(APB);
    pt_write(RESET, pt_read(RESET) | bits);
    pt_write(RESET, pt_read(RESET) & ~bits);
}

static void values(const struct pt_config *c)
{
    pt_write(T3 + 0x28u, c->psc);
    pt_write(T3 + 0x2cu, c->arr);
    pt_write(T3 + 0x34u, c->ccr[0]);
    pt_write(T3 + 0x38u, c->ccr[1]);
}

int pt_f439_start(const struct pt_config *c)
{
    struct pt_ratio f, t, d[2];
    if (pt_ratios(c, &f, &t, d) != PT_OK)
        return PT_INVALID;
    if (generation)
        return PT_BUSY;
    gate(2u);
    pt_write(T3, 0x80u | (c->center ? 0x20u : 0u));
    pt_write(T3 + 0x18u, PT_TARGET_MUTANT == 1 ? 0x6060u : 0x6868u);
    values(c);
    pt_write(T3 + 0x14u, 1u);
    pt_write(T3 + 0x10u, 0u);
    pt_write(T3 + 0x20u, 0x11u);
    configured_clock = c->clock;
    generation = c->generation;
    mode = c->center;
    pt_write(T3, 0x81u | (c->center ? 0x20u : 0u));
    return PT_OK;
}

int pt_f439_stage(const struct pt_config *c, uint32_t current)
{
    struct pt_ratio f, t, d[2];
    uint32_t control;
    if (pt_ratios(c, &f, &t, d) != PT_OK)
        return PT_INVALID;
    if (!generation || c->generation != generation || current != generation ||
        c->clock != configured_clock)
        return PT_STALE;
    if (mode != c->center)
        return PT_BUSY;
    control = pt_read(T3);
    if (!(control & 1u) || (control & 2u))
        return PT_BUSY;
    /* Caller excludes all other register writers. UDIS prevents mixed latching.
     */
    if (PT_TARGET_MUTANT != 2)
        pt_write(T3, control | 2u);
    values(c);
    pt_write(T3, control);
    return PT_OK;
}

void pt_f439_stop(void)
{
    pt_write(T3, pt_read(T3) & ~1u);
    pt_write(T3 + 0x20u, 0u);
    generation = 0;
    configured_clock = 0;
    mode = 0;
}

int pt_f439_capture_start(unsigned psc)
{
    if (psc > 65535u)
        return PT_INVALID;
    gate(4u);
    pt_write(T4, 0u);
    pt_write(T4 + 0x28u, psc);
    pt_write(T4 + 0x2cu, 65535u);
    pt_write(T4 + 0x18u, 1u);
    pt_write(T4 + 0x14u, 1u);
    pt_write(T4 + 0x10u, 0u);
    pt_write(T4 + 0x20u, 1u);
    pt_write(T4, 1u);
    return PT_OK;
}

int pt_f439_capture(unsigned *out)
{
    uint32_t value, status;
    if (!out)
        return PT_INVALID;
    if (!(pt_read(T4 + 0x10u) & 2u))
        return PT_BUSY;
    if (PT_TARGET_MUTANT == 3)
    {
        status = pt_read(T4 + 0x10u);
        value = pt_read(T4 + 0x34u);
    }
    else
    {
        value = pt_read(T4 + 0x34u);
        status = pt_read(T4 + 0x10u);
    }
    if (status & 0x200u)
    {
        /* rc_w0, not W1C; preserve all other implemented status bits. */
        pt_write(T4 + 0x10u, 0x1c5fu);
        return PT_LOST;
    }
    *out = (unsigned)(value & 65535u);
    return PT_OK;
}

static void af2(uintptr_t port, unsigned pin)
{
    uint32_t shift = pin * 2u, v;
    v = pt_read(port + 0x20u);
    v &= ~(15u << (pin * 4u));
    pt_write(port + 0x20u, v | (2u << (pin * 4u)));
    pt_write(port + 4u, pt_read(port + 4u) & ~(1u << pin));
    pt_write(port + 8u, pt_read(port + 8u) & ~(3u << shift));
    pt_write(port + 12u, pt_read(port + 12u) & ~(3u << shift));
    pt_write(port, (pt_read(port) & ~(3u << shift)) | (2u << shift));
}

void pt_f439_pins(void)
{
    pt_write(0x40023830u, pt_read(0x40023830u) | 3u);
    (void)pt_read(0x40023830u);
    af2(0x40020000u, 6u);
    af2(0x40020000u, 7u);
    af2(0x40020400u, 6u);
}
