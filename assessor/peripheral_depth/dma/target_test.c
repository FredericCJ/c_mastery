#include "dma.h"
#include <stdio.h>
#ifndef PD_BOARD
#define PD_BOARD 0
#endif
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("PD_TARGET_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)

static struct
{
    uint32_t a, v;
} r[24];

static unsigned used, cache_ops, clear_writes, clear_reads;

static uint32_t *reg(uint32_t a)
{
    for (unsigned i = 0; i < used; i++)
        if (r[i].a == a)
            return &r[i].v;
    r[used].a = a;
    return &r[used++].v;
}

uint32_t pd_reg_read(uint32_t a)
{
    if (a == (PD_BOARD ? 0x40020408u : 0x40026408u))
        clear_reads++;
    return *reg(a);
}

void pd_reg_write(uint32_t a, uint32_t v)
{
    if (a == (PD_BOARD ? 0x40020408u : 0x40026408u))
    {
        *reg(a - 8) &= ~v;
        clear_writes++;
    }
    *reg(a) = v;
}

void pd_cache_observe(unsigned op, uint32_t a, uint32_t n)
{
    (void)a;
    (void)n;
    cache_ops |= 1u << op;
}

int main(void)
{
    struct pd_port p = {0};
    struct pd_io io = pd_target_io(&p);
    CHECK(io.configure && io.snapshot && io.enable && io.stop && io.visibility);
    uint32_t base = PD_BOARD ? 0x40020400u : 0x40026400u;
    uint32_t mem = PD_BOARD ? 0x30040100u : 0x20000000u;
    struct pd_extent e = {mem, mem + 8192, 64, 64, 64, 4};
    *reg(base) = 0xffffffffu;
    CHECK(io.configure(io.context, &e, 7) && *reg(base + 0x18) == mem &&
          *reg(base + 0x1c) == mem + 8192 && *reg(base + 0x14) == 16 &&
          *reg(base + 0x24) == 7 && *reg(base + 0x10) == 0x5680);
    CHECK(clear_writes == 1 && !clear_reads && (*reg(base) & 0x3d) == 0 &&
          (*reg(base) & 0x80000000u));
    io.visibility(io.context, PD_RELEASE, 0, 0);
    io.visibility(io.context, PD_CLEAN, mem, 64);
    io.visibility(io.context, PD_INVALIDATE, mem + 8192, 64);
    CHECK(cache_ops == 13);
    io.enable(io.context);
    CHECK(*reg(base + 0x10) == 0x5681);
    struct pd_snapshot x = io.snapshot(io.context);
    CHECK(x.enabled == 1 && !(x.flags & PD_TC));
    CHECK(!io.configure(io.context, &e, 8) && p.token == 7);
    io.stop(io.context);
    CHECK(!(*reg(base + 0x10) & 1));
    *reg(base) = PD_TC;
    *reg(base + 0x24) = 0x27;
    *reg(base + 0x14) = 3;
    x = io.snapshot(io.context);
    CHECK(x.token == 7 && !x.enabled && x.remaining == 3 && x.flags == PD_TC &&
          x.fifo_empty);
    puts("PD_TARGET_PASS");
    return 0;
}
