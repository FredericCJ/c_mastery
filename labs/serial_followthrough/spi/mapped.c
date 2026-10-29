#include "scheduler.h"
void ss_note(const char *);
void ss_values(const char *, unsigned, unsigned, unsigned, unsigned);
#define CHECK(x)                           \
    do                                     \
    {                                      \
        if (!(x))                          \
        {                                  \
            ss_note("SS_MAP_ASSERT: " #x); \
            return 41;                     \
        }                                  \
    } while (0)
static uint32_t addresses[64], values[64];
static unsigned used;

volatile uint32_t *ps_test_register(uint32_t address)
{
    for (unsigned i = 0; i < used; i++)
    {
        if (addresses[i] == address)
        {
            return values + i;
        }
    }
    if (used == 64)
    {
        return 0;
    }
    addresses[used] = address;
    return values + used++;
}

#define R(a) (*ps_test_register(a))
#define SPI 0x40013000u
#define RX 0x40026410u
#define TX 0x40026458u

int ss_mapped(void)
{
    struct ps_spi s = {0};
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    struct ps_io io = ps_f439_io();
    R(0x40020020u) = 0xffffffffu;
    ps_f439_pins();
    CHECK(R(0x40020020u) == 0x555fffffu);
    CHECK(ps_open(&s, io, &c, 3, 3) == PS_OK && R(SPI) == 0x35fu);
    CHECK(ps_f439_begin_dma(&s, 1, 1, (const uint16_t *)(uintptr_t)0x10000000u,
                            (uint16_t *)(uintptr_t)0x20000100u, 3, 0, 100, 0,
                            2) == PS_INVALID);
    CHECK(ps_f439_begin_dma(&s, 1, 1, (const uint16_t *)(uintptr_t)0x2002fffcu,
                            (uint16_t *)(uintptr_t)0x20000100u, 3, 0, 100, 0,
                            2) == PS_INVALID);
    CHECK(ps_f439_begin_dma(&s, 1, 1, (const uint16_t *)(uintptr_t)0x20000000u,
                            (uint16_t *)(uintptr_t)0x20000100u, 3, 0, 100, 0,
                            2) == PS_OK);
    R(SPI + 8) = PS_TXE;
    CHECK(ps_step(&s, io, 1, 0) == PS_BUSY);
    CHECK(R(RX) == 0x06002c01u && R(TX) == 0x06002c41u && R(RX + 4) == 3 &&
          R(TX + 4) == 3);
    CHECK(R(RX + 12) == 0x20000100u && R(TX + 12) == 0x20000000u &&
          R(RX + 8) == SPI + 12 && R(TX + 8) == SPI + 12);
    /* Actual controller adapter writes request-disable; only later model EN reads acknowledge it. */
    R(SPI + 8) = PS_TXE | PS_BSY;
    CHECK(ps_cancel(&s, io, 1) == PS_BUSY);
    R(RX) |= 1;
    R(TX) |= 1;
    CHECK(ps_step(&s, io, 1, 1) == PS_BUSY && s.tx_owned && s.rx_owned &&
          s.cs_owned);
    R(TX) &= ~1u;
    CHECK(ps_step(&s, io, 1, 2) == PS_BUSY && !s.tx_owned && s.rx_owned);
    R(RX) &= ~1u;
    CHECK(ps_step(&s, io, 1, 3) == PS_BUSY && !s.rx_owned && s.cs_owned);
    R(SPI + 8) = PS_TXE;
    CHECK(ps_step(&s, io, 1, 4) == PS_BUSY && s.cs_owned);
    CHECK(ps_step(&s, io, 1, 6) == PS_CANCELLED &&
          R(0x40020c18u) == (1u << 14));
    ss_values("mapped", 0x06002c01u, 0x06002c41u, 3, s.result);
    /* Compose the scheduler's real slot addresses with the actual F439 adapter.
     * Native process storage is outside the allowed SRAM; target storage is in it. */
    static struct ss_scheduler q;
    uint8_t bytes[3] = {0xc3, 0, 0}, out[16] = {0};
    struct ss_result result = {99, 99, 99};
    CHECK(ss_open(&q, io, &c, 0, 3) == PS_OK &&
          ss_enqueue(&q, 1, bytes, 3, 1, 100, 0, 0) == PS_OK);
    int admitted = ss_run(&q, io, ss_f439_submit, 10);
#if defined(__arm__) || defined(__thumb__)
    CHECK(admitted == PS_BUSY);
    R(SPI + 8) = PS_TXE;
    CHECK(ss_event(&q, io, 1, 11) == PS_BUSY);
    CHECK(R(RX + 12) == (uintptr_t)q.slots[0].rx_words &&
          R(TX + 12) == (uintptr_t)q.slots[0].tx_words);
    CHECK(ss_cancel(&q, io, 1) == PS_BUSY);
    R(RX) &= ~1u;
    R(TX) &= ~1u;
    CHECK(ss_event(&q, io, 1, 12) == PS_CANCELLED &&
          ss_take(&q, 1, &result, out) == PS_OK);
    CHECK(result.status == PS_CANCELLED);
    ss_values("mapped_scheduler", 1, 6, 2, 1);
#else
    CHECK(admitted == PS_INVALID && ss_take(&q, 1, &result, out) == PS_OK &&
          result.status == PS_INVALID && out[0] == 0);
    ss_values("mapped_scheduler", 0, 6, 2, 1);
#endif
    return 0;
}
