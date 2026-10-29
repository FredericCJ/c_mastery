#include "target.h"
void en_note(const char *);
#define CHECK(x)                            \
    do                                      \
    {                                       \
        if (!(x))                           \
        {                                   \
            en_note("EN_PORT_ASSERT: " #x); \
            return 41;                      \
        }                                   \
    } while (0)
static uint32_t gpio[7][10], rcc[18], pmc, demcr, ctrl, count, eth[1032];
static unsigned writes, violation, frozen, absent, reads, barriers;

uint32_t en_port_load(uint32_t a)
{
    if (a == 0xe000edfc)
    {
        return demcr;
    }
    if (a == 0xe0001000)
    {
        return ctrl | (absent ? 0x2000000u : 0);
    }
    if (a == 0xe0001004)
    {
        reads++;
        if (!frozen && (ctrl & 1u) && (demcr & 0x1000000u))
        {
            count += 8;
        }
        return count;
    }
    if (a >= 0x40028000u && a <= 0x4002901cu)
    {
        return eth[(a - 0x40028000u) / 4];
    }
    if (a >= 0x40023800u && a <= 0x40023844u)
    {
        return rcc[(a - 0x40023800u) / 4];
    }
    if (a == 0x40013804)
    {
        return pmc;
    }
    if (a >= 0x40020000u && a < 0x40021c00u)
    {
        unsigned p = (a - 0x40020000u) / 0x400u, o = (a - 0x40020000u) % 0x400u;
        if (o <= 0x24)
        {
            return gpio[p][o / 4];
        }
    }
    violation++;
    return 0;
}

void en_port_store(uint32_t a, uint32_t v)
{
    writes++;
    if (a == 0xe000edfc)
    {
        demcr = v;
        return;
    }
    if (a == 0xe0001000)
    {
        ctrl = v;
        return;
    }
    if (a >= 0x40028000u && a <= 0x4002901cu)
    {
        eth[(a - 0x40028000u) / 4] = v;
        return;
    }
    if (a >= 0x40023800u && a <= 0x40023844u)
    {
        rcc[(a - 0x40023800u) / 4] = v;
        return;
    }
    if (a == 0x40013804)
    {
        if (!(rcc[0x10 / 4] & 0x2000000u) || (rcc[0x30 / 4] & 0xe000000u))
        {
            violation++;
        }
        pmc = v;
        return;
    }
    if (a >= 0x40020000u && a < 0x40021c00u)
    {
        unsigned p = (a - 0x40020000u) / 0x400u, o = (a - 0x40020000u) % 0x400u;
        if (o <= 0x24)
        {
            if (!(rcc[0x30 / 4] & (1u << p)))
            {
                violation++;
            }
            gpio[p][o / 4] = v;
            return;
        }
    }
    violation++;
}

void en_port_barrier(void)
{
    barriers++;
}

int en_mapped(void)
{
    for (unsigned i = 0; i < 7; i++)
    {
        for (unsigned j = 0; j < 10; j++)
        {
            gpio[i][j] = 0;
        }
    }
    for (unsigned i = 0; i < 18; i++)
    {
        rcc[i] = 0;
    }
    pmc = 0;
    demcr = 0x10000;
    ctrl = 0;
    count = UINT32_MAX - 100;
    writes = violation = frozen = reads = barriers = 0;
    struct en_target t = {0};
    absent = 1;
    CHECK(en_target_open(&t) == ED_INVALID && !t.opened && demcr == 0x10000);
    absent = 0;
    CHECK(en_target_open(&t) == ED_OK && en_target_cold(&t) == ED_OK &&
          !violation);
    CHECK(pmc == 0x800000u && (rcc[0x30 / 4] & 0xe000047u) == 0xe000047u &&
          !(rcc[0x10 / 4] & 0x2000000u));
    unsigned ports[] = {0, 0, 0, 1, 2, 2, 2, 6, 6},
             pins[] = {1, 2, 7, 13, 1, 4, 5, 11, 13};
    for (unsigned i = 0; i < 9; i++)
    {
        unsigned p = ports[i], n = pins[i];
        CHECK(((gpio[p][n < 8 ? 8 : 9] >> ((n % 8) * 4)) & 15u) == 11 &&
              ((gpio[p][0] >> (n * 2)) & 3u) == 2 &&
              ((gpio[p][2] >> (n * 2)) & 3u) == 2 && !(gpio[p][1] & (1u << n)));
    }
    CHECK(gpio[0][0] == 0x8028u && gpio[1][0] == 0x08000000u &&
          gpio[2][0] == 0xa08u && gpio[6][0] == 0x08800000u);
    unsigned before = writes;
    CHECK(en_target_cold(&t) == ED_BUSY && writes == before);
    struct en_bus b = en_target_bus(&t);
    uint32_t start = count;
    CHECK(b.settle(b.context) == 0 && (uint32_t)(count - start) >= 720u);
    CHECK(b.write(b.context, 0x1018, 0x02202002) == 0);
    uint32_t out = 0;
    CHECK(b.read(b.context, 0x1018, &out) == 0 && out == 0x02202002u);
    frozen = 1;
    reads = 0;
    CHECK(b.settle(b.context) != 0 && reads == 2049);
    CHECK(b.read(b.context, 1, &out) != 0 &&
          b.write(b.context, 0x1020, 0) != 0 && !violation);
    CHECK(en_target_close(&t) == ED_OK && demcr == 0x10000 && ctrl == 0 &&
          !t.opened && b.settle(b.context) != 0);
    en_note("OBS ring_port 9 720 2049 0");
    return 0;
}
