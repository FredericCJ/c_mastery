#include "channel.h"
#ifndef R5_TARGET_MODEL
_Static_assert(sizeof(uintptr_t) == 4, "F439 address width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))

static void barrier(void)
{
    __asm__ volatile("dsb" ::: "memory");
}
#else
volatile uint32_t *r5_test_register(uint32_t);
void r5_test_barrier(void);
#define REG(a) (*r5_test_register(a))

static void barrier(void)
{
    r5_test_barrier();
}
#endif
#ifndef R5_PORT_MUTANT
#define R5_PORT_MUTANT 0
#endif
void r5_f439_prepare(void)
{
    pu_f439_pins();
    /* Exclusive GPIOD: initial inactive output latch before output mode. */
    REG(0x40020c18u) = 1u << 26;
    REG(0x40020c04u) &= ~(1u << 10);
    REG(0x40020c08u) &= ~(3u << 20);
    REG(0x40020c0cu) &= ~(3u << 20);
    REG(0x40020c00u) = (REG(0x40020c00u) & ~(3u << 20)) | (1u << 20);
    barrier();
}

static void drive(void *p, unsigned on)
{
    (void)p;
    REG(0x40020c18u) =
        1u << (R5_PORT_MUTANT == 1 ? (on ? 26 : 10) : (on ? 10 : 26));
    barrier();
}

struct r5_io r5_f439_io(void)
{
    struct r5_io io = {pu_f439_io(), 0, drive};
    return io;
}

int r5_f439_send(struct r5_channel *s, uint32_t token, const uint8_t *data,
                 size_t n, unsigned backend, uint32_t now, uint32_t budget,
                 size_t *accepted)
{
    if (!s)
    {
        return R5_INVALID;
    }
    uintptr_t p = (uintptr_t)s->tx_bytes;
    if (backend == PU_DMA &&
        (p < 0x20000000u || p >= 0x20030000u || 16 > 0x20030000u - p))
    {
        return R5_INVALID;
    }
    return r5_send(s, token, data, n, backend, now, budget, accepted);
}
