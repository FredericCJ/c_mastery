#include "firmware_io.h"
#include "serial_depth.h"
#include "spi.h"
#include <stdio.h>
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            printf("PS_COMPOSE_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                            \
        }                                                        \
    } while (0)
static unsigned phase, busy, cs, read_count;
static uint8_t peer;

static uint32_t get(void *p, unsigned r)
{
    (void)p;
    if (r == FW_DATA)
    {
        ++read_count;
        return peer;
    }
    if (r == FW_STATUS)
    {
        return phase ? FW_TXE | FW_RXNE | (busy ? FW_BUSY : 0) : FW_TXE;
    }
    return 0;
}

static void put(void *p, unsigned r, uint32_t v)
{
    (void)p;
    if (r == FW_CONTROL)
    {
        cs = v;
    }
    if (r == FW_DATA)
    {
        phase = 1;
    }
}

static uint32_t sr[19];

static uint32_t psget(void *p, unsigned r)
{
    (void)p;
    if (r == PS_DR)
    {
        sr[PS_SR] &= ~PS_RXNE;
    }
    return sr[r];
}

static void psput(void *p, unsigned r, uintptr_t v)
{
    (void)p;
    sr[r] = (uint32_t)v;
    if (r == PS_DR)
    {
        sr[PS_SR] = PS_BSY;
    }
}

int main(void)
{
    struct fw_io fw = {0, get, put};
    struct pu_clock clock = {16000000, 1, 1, 1, 1, 0};
    struct ps_io io = {0, psget, psput};
    for (unsigned n = 0; n < 256; n++)
    {
        struct ps_spi s = {0};
        struct sd_spi serial = {0};
        uint8_t tx = (uint8_t)n, rx = 0, old = 0;
        peer = (uint8_t)(n ^ 0xa5u);
        phase = busy = read_count = 0;
        CHECK(!fw_spi(fw, tx, &old, 3) && old == peer && read_count == 1 &&
              !cs);
        sr[PS_SR] = 0;
        CHECK(!ps_open(&s, io, &clock, 0, 3));
        CHECK(!ps_begin(&s, io, 1, 1, &tx, &rx, 1, 0, 0, 10, 0, 0));
        sr[PS_SR] = PS_TXE;
        CHECK(ps_step(&s, io, 1, 0) == PS_BUSY);
        sr[PS_SR] = PS_TXE | PS_RXNE;
        sr[PS_DR] = peer;
        CHECK(ps_step(&s, io, 1, 1) == PS_OK && rx == old);
        CHECK(sd_spi_begin(&serial, 1, 0, 1) == SD_OK);
        CHECK(sd_spi_poll(&serial, 1, 1, 0, 1, 0) == SD_BUSY);
        CHECK(sd_spi_poll(&serial, 1, 1, 1, 0, peer) == SD_DONE &&
              serial.rx[0] == old);
    }
    /* Actual frozen baseline drops CS on a busy timeout. Preserve its history;
       it cannot supply the new held-CS contract without this successor. */
    {
        uint8_t value = 77;
        phase = read_count = 0;
        busy = 1;
        CHECK(fw_spi(fw, 9, &value, 2) == -1 && value == 77 && !cs &&
              read_count == 1);
    }
    puts("PS_COMPOSE_PASS 256 equivalent completed bytes; frozen fw_spi "
         "busy-timeout CS counterexample");
    return 0;
}
