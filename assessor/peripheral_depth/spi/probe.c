#include "serial_depth.h"
#include "spi.h"
#include <stdio.h>
static uint32_t regs[19];

static uint32_t rd(void *p, unsigned r)
{
    (void)p;
    return regs[r];
}

static void wr(void *p, unsigned r, uintptr_t v)
{
    (void)p;
    if (r == PS_TX_EN || r == PS_RX_EN)
        return;
    regs[r] = (uint32_t)v;
    if (r == PS_TX_START)
        regs[PS_TX_EN] = 1;
    if (r == PS_RX_START)
        regs[PS_RX_EN] = 1;
}

int main(void)
{
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    struct ps_io io = {0, rd, wr};
    for (unsigned mode = 0; mode < 4; mode++)
        for (unsigned tx = 0; tx < 256; tx++)
        {
            uint8_t level[16], sample[16], mosi[16], rx;
            struct ps_spi s = {0};
            regs[PS_SR] = 0;
            if (ps_open(&s, io, &c, mode, 3) ||
                sd_spi_edge(mode, (uint8_t)tx, (uint8_t)(tx ^ 0xa5u), level,
                            sample, mosi, &rx))
                return 1;
            printf("E %u %u %u %u", mode, tx, regs[PS_CONFIG], rx);
            for (unsigned i = 0; i < 16; i++)
                printf(" %u %u %u", level[i], sample[i], mosi[i]);
            puts("");
        }
    for (unsigned version = 0; version < 2; version++)
        for (unsigned value = 0; value < 65536; value++)
        {
            uint8_t in[3] = {(uint8_t)(version ? 0xa6 : 0xa5), (uint8_t)value,
                             (uint8_t)(value >> 8)};
            uint16_t result = 0;
            int r = ps_decode(version, in, &result);
            printf("P %u %u %d %u\n", version, value, r, result);
        }
    for (unsigned error = 0; error < 256; error++)
        for (unsigned version = 0; version < 2; version++)
        {
            uint8_t in[3] = {(uint8_t)error, 0xef, 0x40};
            uint16_t result = 777;
            int r = ps_decode(version, in, &result);
            printf("F %u %u %d %u\n", version, error, r, result);
        }
    for (unsigned value = 0; value < 65536; value++)
    {
        uint8_t v = (uint8_t)value, out = 0;
        uint16_t word = 0, wide = (uint16_t)value;
        if (ps_pack(&v, &word, 1) || ps_unpack(&wide, &out, 1))
            return 1;
        printf("W %u %u %u\n", value, word, out);
    }
    /* All relative orders of two disable acknowledgements and wire idle;
       ties allowed, four holds, ordinary and wrapping clocks, cancel/success. */
    for (unsigned cancel = 0; cancel < 2; cancel++)
        for (unsigned wrap = 0; wrap < 2; wrap++)
            for (unsigned hold = 0; hold < 4; hold++)
                for (unsigned tx = 1; tx < 5; tx++)
                    for (unsigned rx = 1; rx < 5; rx++)
                        for (unsigned wire = 1; wire < 5; wire++)
                        {
                            struct ps_spi s = {0};
                            uint8_t a = 9, b = 0;
                            uint32_t start = wrap ? UINT32_MAX - 3u : 0;
                            for (unsigned i = 0; i < 19; i++)
                                regs[i] = 0;
                            if (ps_open(&s, io, &c, 0, 3) ||
                                ps_begin(&s, io, 1, 1, &a, &b, 1, 1, start, 20,
                                         0, hold))
                                return 1;
                            regs[PS_SR] = PS_BSY;
                            if (ps_step(&s, io, 1, start) != PS_BUSY)
                                return 1;
                            if (cancel && ps_cancel(&s, io, 1) != PS_BUSY)
                                return 1;
                            for (unsigned t = 1; t <= 7; t++)
                            {
                                if (t >= tx)
                                {
                                    regs[PS_TX_EN] = 0;
                                    regs[PS_TX_FLAGS] = 1;
                                    regs[PS_TX_COUNT] = 0;
                                }
                                if (t >= rx)
                                {
                                    regs[PS_RX_EN] = 0;
                                    regs[PS_RX_FLAGS] = 1;
                                    regs[PS_RX_COUNT] = 0;
                                }
                                regs[PS_SR] = t >= wire ? PS_TXE : PS_BSY;
                                int r = ps_step(&s, io, 1, start + t);
                                printf(
                                    "S %u %u %u %u %u %u %u %d %u %u %u %u %u\n",
                                    cancel, wrap, hold, tx, rx, wire, t, r,
                                    s.tx_owned, s.rx_owned, s.cs_owned, s.sent,
                                    s.received);
                                if (!s.phase)
                                    break;
                            }
                        }
    return 0;
}
