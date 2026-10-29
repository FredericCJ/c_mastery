#include "spi.h"
#ifndef PS_TARGET_MUTANT
#define PS_TARGET_MUTANT 0
#endif
#ifdef PS_TARGET_MODEL
volatile uint32_t *ps_test_register(uint32_t);
#define REG(a) (*ps_test_register(a))
#else
_Static_assert(sizeof(uintptr_t) == 4, "F439 address width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#endif
#define SPI 0x40013000u
#define RX 0x40026410u
#define TX 0x40026458u

static void dma_barrier(void)
{
#ifndef PS_TARGET_MODEL
    __asm__ volatile("dmb sy" ::: "memory");
#endif
}

static uint32_t read_reg(void *context, unsigned r)
{
    (void)context;
    switch (r)
    {
        case PS_SR:
            return REG(SPI + 8u);
        case PS_DR:
            return REG(SPI + 12u) & 255u;
        case PS_TX_EN:
        {
            uint32_t enabled = REG(TX) & 1u;
            if (!enabled)
            {
                dma_barrier();
            }
            return enabled;
        }
        case PS_RX_EN:
        {
            uint32_t enabled = REG(RX) & 1u;
            if (!enabled)
            {
                dma_barrier();
            }
            return enabled;
        }
        case PS_TX_COUNT:
            return REG(TX + 4u);
        case PS_RX_COUNT:
            return REG(RX + 4u);
        case PS_TX_FLAGS:
        {
            uint32_t x = REG(0x40026400u);
            return ((x >> 27) & 1u) |
                   ((x & (1u << 22 | 1u << 24 | 1u << 25)) ? 2u : 0u);
        }
        case PS_RX_FLAGS:
        {
            uint32_t x = REG(0x40026400u);
            return ((x >> 5) & 1u) | ((x & (1u | 1u << 2 | 1u << 3)) ? 2u : 0u);
        }
        default:
            return 0;
    }
}

static void write_reg(void *context, unsigned r, uintptr_t value)
{
    uint32_t v = (uint32_t)value;
    (void)context;
    switch (r)
    {
        case PS_DR:
            REG(SPI + 12u) = v & 255u;
            break;
        case PS_CS:
            REG(0x40020c18u) = 1u << (v ? 14u : 30u);
            break;
        case PS_CONFIG:
            REG(SPI) = v;
            REG(SPI + 4u) = 0;
            break;
        case PS_ENABLE:
            if (v)
            {
                REG(SPI) |= 64u;
            }
            else
            {
                REG(SPI) &= ~64u;
            }
            break;
        case PS_TX_EN:
            if (!v)
            {
                REG(TX) &= ~1u;
            }
            break;
        case PS_RX_EN:
            if (!v)
            {
                REG(RX) &= ~1u;
            }
            break;
        case PS_TX_COUNT:
            REG(TX + 4u) = v;
            break;
        case PS_RX_COUNT:
            REG(RX + 4u) = v;
            break;
        case PS_TX_ADDR:
            REG(TX + 12u) = v;
            break;
        case PS_RX_ADDR:
            REG(RX + 12u) = v;
            break;
        case PS_DMA_CLEAR:
            REG(0x40026408u) = 0x0f40003du;
            break;
        case PS_RX_START:
            dma_barrier();
            REG(RX + 8u) = SPI + 12u;
            REG(RX + 20u) = 0;
            REG(RX) = (PS_TARGET_MUTANT == 1 ? 4u : 3u) << 25 |
                      (PS_TARGET_MUTANT == 3 ? 0u : (1u << 13 | 1u << 11)) |
                      1u << 10 | 1u;
            break;
        case PS_TX_START:
            dma_barrier();
            REG(TX + 8u) = SPI + 12u;
            REG(TX + 20u) = 0;
            REG(TX) = 3u << 25 | 1u << 13 | 1u << 11 | 1u << 10 | 1u << 6 | 1u;
            break;
        case PS_DMA_REQUESTS:
            REG(SPI + 4u) = (REG(SPI + 4u) & ~3u) | (v & 3u);
            break;
        case PS_RESET:
            REG(0x40023824u) |= 1u << 12;
            (void)REG(0x40023824u);
            REG(0x40023824u) &= ~(1u << 12);
            break;
        default:
            break;
    }
}

struct ps_io ps_f439_io(void)
{
    struct ps_io io = {0, read_reg, write_reg};
    return io;
}

void ps_f439_pins(void)
{
    /* One-time exclusive preparation before ps_open, never during a transfer. */
    REG(0x40023830u) |= (1u << 22) | 9u;
    (void)REG(0x40023830u);
    REG(0x40023844u) |= 1u << 12;
    (void)REG(0x40023844u);
    REG(0x40023824u) |= 1u << 12;
    REG(0x40023824u) &= ~(1u << 12);
    REG(0x40020c18u) = 1u << 14;
    REG(0x40020c04u) &= ~(1u << 14);
    REG(0x40020c08u) &= ~(3u << 28);
    REG(0x40020c0cu) &= ~(3u << 28);
    REG(0x40020c00u) = (REG(0x40020c00u) & ~(3u << 28)) | (1u << 28);
    REG(0x40020020u) = (REG(0x40020020u) & ~0xfff00000u) |
                       (PS_TARGET_MUTANT == 2 ? 0x44400000u : 0x55500000u);
    REG(0x40020004u) &= ~0xe0u;
    REG(0x40020008u) = (REG(0x40020008u) & ~0xfc00u) | 0xa800u;
    REG(0x4002000cu) &= ~0xfc00u;
    REG(0x40020000u) = (REG(0x40020000u) & ~0xfc00u) | 0xa800u;
}

int ps_f439_begin(struct ps_spi *s, uint32_t generation, uint32_t token,
                  const uint8_t *tx, uint8_t *rx, unsigned count, unsigned dma,
                  uint32_t now, unsigned budget, unsigned setup, unsigned hold)
{
    uintptr_t a = (uintptr_t)tx, b = (uintptr_t)rx;
    if (!count || count > 16 || dma)
    {
        return PS_INVALID;
    }
    if (dma && (a < 0x20000000u || b < 0x20000000u || a >= 0x20030000u ||
                b >= 0x20030000u || count > 0x20030000u - a ||
                count > 0x20030000u - b))
    {
        return PS_INVALID;
    }
    if ((a <= b && b - a < count) || (b < a && a - b < count))
    {
        return PS_INVALID;
    }
    return ps_begin(s, ps_f439_io(), generation, token, tx, rx, count, dma, now,
                    budget, setup, hold);
}

int ps_f439_begin_dma(struct ps_spi *s, uint32_t generation, uint32_t token,
                      const uint16_t *tx, uint16_t *rx, unsigned count,
                      uint32_t now, unsigned budget, unsigned setup,
                      unsigned hold)
{
    uintptr_t a = (uintptr_t)tx, b = (uintptr_t)rx;
    unsigned bytes;
    if (!count || count > 16)
    {
        return PS_INVALID;
    }
    bytes = 2u * count;
    if ((a & 1u) || (b & 1u) || a < 0x20000000u || b < 0x20000000u ||
        a >= 0x20030000u || b >= 0x20030000u || bytes > 0x20030000u - a ||
        bytes > 0x20030000u - b)
    {
        return PS_INVALID;
    }
    if ((a <= b && b - a < bytes) || (b < a && a - b < bytes))
    {
        return PS_INVALID;
    }
    /* Direct mode has equal halfword memory/peripheral accesses. DFF remains8. */
    return ps_begin(s, ps_f439_io(), generation, token, (const uint8_t *)tx,
                    (uint8_t *)rx, count, 1, now, budget, setup, hold);
}
