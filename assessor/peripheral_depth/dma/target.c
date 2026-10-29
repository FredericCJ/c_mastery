#include "dma.h"
#ifndef PD_BOARD
#define PD_BOARD PD_F439
#endif
#ifndef PD_TARGET_MUTANT
#define PD_TARGET_MUTANT 0
#endif
/* PD_BOARD is numeric0(F439 M4) or1(H745 M7), not an enum in #if. */
#ifdef PD_TARGET_MODEL
uint32_t pd_reg_read(uint32_t);
void pd_reg_write(uint32_t, uint32_t);
void pd_cache_observe(unsigned, uint32_t, uint32_t);
#define RD(a) pd_reg_read(a)
#define WR(a, v) pd_reg_write(a, v)
#else
_Static_assert(sizeof(uintptr_t) == 4, "32-bit selected target");
#define RD(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define WR(a, v) (*(volatile uint32_t *)(uintptr_t)(a) = (v))
#if PD_BOARD == 1
#include "h745_cache.h"
#endif
#endif
#if PD_BOARD == 1
#define DMA 0x40020400u
#define GATE 0x580244d8u
#define ENABLE 2u
#else
#define DMA (PD_TARGET_MUTANT == 4 ? 0x40026000u : 0x40026400u)
#define GATE 0x40023830u
#define ENABLE (1u << 22)
#endif
static void visibility(void *ctx, unsigned op, uint32_t address,
                       uint32_t length)
{
    (void)ctx;
#ifdef PD_TARGET_MODEL
    pd_cache_observe(op, address, length);
#else
    if (op == PD_RELEASE || op == PD_ACQUIRE)
    {
        __asm__ volatile("dsb sy" ::: "memory");
        return;
    }
#if PD_BOARD == 1
    (void)fw_h745_cache(address, length,
                        op == PD_CLEAN ? FW_CLEAN : H7_INVALIDATE);
#else
    (void)address;
    (void)length;
#endif
#endif
}

static int configure(void *ctx, const struct pd_extent *e, uint32_t token)
{
    struct pd_port *p = ctx;
    uint32_t total;
    if (!p || !token || pd_validate(PD_BOARD, PD_BOARD, e, 1, &total) != PD_OK)
        return 0;
    WR(GATE, RD(GATE) | ENABLE);
    (void)RD(GATE);
    if (RD(DMA + 0x10) & 1)
        return 0;
    WR(DMA + 8, 0x3d); /* W1C stream0 only; never a read-modify-write. */
    WR(DMA + 0x18, PD_TARGET_MUTANT == 1 ? e->destination : e->source);
    WR(DMA + 0x1c, e->destination);
    WR(DMA + 0x14, e->bytes / e->width);
    WR(DMA + 0x24, PD_TARGET_MUTANT == 2 ? 0u : 7u); /* M2M requires FIFO. */
    unsigned size = e->width == 4 ? 2 : e->width == 2 ? 1 : 0;
    p->configuration = 0x680u | (size << 11) | (size << 13);
    p->token = token;
    WR(DMA + 0x10, p->configuration);
    return 1;
}

static void enable(void *ctx)
{
    struct pd_port *p = ctx;
    WR(DMA + 0x10, p->configuration | 1);
}

static void stop(void *ctx)
{
    (void)ctx;
    WR(DMA + 0x10, RD(DMA + 0x10) & ~1u);
}

static struct pd_snapshot snapshot(void *ctx)
{
    struct pd_port *p = ctx;
    struct pd_snapshot x = {p->token, 0, 0, 0, 0};
    x.enabled = RD(DMA + 0x10) & 1;
    x.flags = RD(DMA) & 0x3d;
    if (!x.enabled)
    {
        x.fifo_empty = (RD(DMA + 0x24) & 0x38) == 0x20;
        x.remaining = RD(DMA + 0x14);
    }
    if (PD_TARGET_MUTANT == 3)
    {
        x.enabled = 0;
        x.fifo_empty = 1;
        x.flags |= PD_TC;
    }
    return x;
}

struct pd_io pd_target_io(struct pd_port *p)
{
    struct pd_io io = {p, configure, enable, stop, snapshot, visibility};
    return io;
}
