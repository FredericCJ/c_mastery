#include "f439_port.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                        \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "F4_PORT_CHECK:%d:%s\n", __LINE__, #x); \
            return __LINE__;                                        \
        }                                                           \
    } while (0)

struct f4_model
{
    uint32_t a[100], v[100], r[16];
    unsigned used, sync, sr_seen, dr_reads, violations, txstage, txpoll;
};

static uint32_t rd(void *c, uint32_t a)
{
    struct f4_model *m = c;
    if (a == 0x40004400u)
    {
        m->sr_seen = 1;
        if (m->txstage)
        {
            if (++m->txpoll == 3)
                m->r[0] |= 64u;
        }
        return m->r[0];
    }
    if (a == 0x40004404u)
    {
        if (!m->sr_seen)
            ++m->violations;
        m->sr_seen = 0;
        ++m->dr_reads;
        m->r[0] &= ~63u;
        return m->r[1];
    }
    if (a == 0xE0042000u)
        return m->r[9];
    if (a == 0x40023800u)
        return m->r[2];
    if (a == 0x40023808u)
        return m->r[3];
    if (a == 0xE000ED94u)
        return m->r[4];
    if (a == 0x40023C00u)
        return m->r[5];
    if (a == 0x40023C14u)
        return m->r[6];
    if (a == 0x40013800u)
        return m->r[7];
    if (a == 0x4000440Cu)
        return m->r[8];
    for (unsigned i = m->used; i; i--)
        if (m->a[i - 1] == a)
            return m->v[i - 1];
    return 0;
}

static void wr(void *c, uint32_t a, uint32_t v)
{
    struct f4_model *m = c;
    if (m->used < 100)
    {
        m->a[m->used] = a;
        m->v[m->used++] = v;
    }
    else
        ++m->violations;
    if (a == 0x40004404u)
    {
        if (!m->sr_seen)
            ++m->violations;
        m->r[0] &= ~64u;
        m->txstage = 1;
    }
    if (a == 0x40023C00u && ((m->r[5] & 0x600u) || (v & 0x600u)))
        ++m->violations;
}

static void sync_io(void *c)
{
    ++((struct f4_model *)c)->sync;
}

static struct fh_io io(struct f4_model *m)
{
    return (struct fh_io){m, rd, wr, sync_io};
}

int main(void)
{
    struct f4_model m = {.r = {[2] = 3, [9] = 0x20030419}};
    C(f4_cold(io(&m)));
    C(m.sync && m.a[0] == 0x40023844u && (m.v[0] & 0x4000u));
    for (unsigned i = 2; i <= 7; ++i)
    {
        m = (struct f4_model){.r = {[2] = 3, [9] = 0x20030419}};
        m.r[i] =
            i == 2
                ? 0
                : (i == 3
                       ? 4
                       : (i == 4 ? 1 : (i == 5 ? 0x200 : (i == 6 ? 16 : 256))));
        C(!f4_cold(io(&m)));
    }
    const uint32_t badids[] = {0x10000419u, 0x10030419u, 0x10070419u,
                               0x20030413u, 0x30000419u};
    for (unsigned i = 0; i < 5; ++i)
    {
        m = (struct f4_model){.r = {[2] = 3, [9] = badids[i]}};
        C(!f4_cold(io(&m)));
    }
    m = (struct f4_model){.r = {[2] = 3, [9] = 0x20010419}};
    C(f4_cold(io(&m)));
    m = (struct f4_model){.r = {[2] = 3, [9] = 0x20030419}};
    C(f4_uart_init(io(&m)));
    C(rd(&m, 0x40020000) == 0xA0 && rd(&m, 0x40020020) == 0x7700 &&
      rd(&m, 0x40004408) == 139 &&
      rd(&m, 0x4000440C) ==
          0); /* register8 is the separate live fixture control */
    C(m.v[m.used - 1] == 0x200C && !m.violations);
    m.r[8] = 0x200C;
    C(!f4_uart_init(io(&m)));
    for (unsigned error = 1; error <= 8; error *= 2)
    {
        m = (struct f4_model){.r = {32 | error, 0xA5}};
        unsigned char out = 0xCC;
        uint32_t s = 0;
        C(f4_uart_rx(io(&m), &out, &s) == -1 && out == 0xCC &&
          s == (32 | error) && m.dr_reads == 1 && !m.violations);
    }
    m = (struct f4_model){.r = {1, 0xA5}};
    unsigned char out = 0xCC;
    uint32_t s = 0;
    C(f4_uart_rx(io(&m), &out, &s) == -1 && !m.dr_reads && out == 0xCC);
    m = (struct f4_model){.r = {0, 0xA5}};
    C(f4_uart_rx(io(&m), &out, &s) == 0 && !m.dr_reads);
    m.r[0] = 32;
    C(f4_uart_rx(io(&m), &out, &s) == 1 && out == 0xA5 && m.dr_reads == 1);
    m = (struct f4_model){.r = {128 | 64}};
    C(f4_uart_tx(io(&m), 5, 4) && m.txpoll == 3 && m.v[0] == 5 && m.sync);
    m = (struct f4_model){0};
    C(!f4_uart_tx(io(&m), 5, 2) && !m.used);
    m = (struct f4_model){.r = {128}};
    C(!f4_uart_tx(io(&m), 5, 2) && m.used == 1);
    m = (struct f4_model){0};
    f4_fresh(io(&m));
    C(m.used == 2 && m.a[0] == 0x40023C00 && m.v[0] == 0x1800 && m.v[1] == 0 &&
      m.sync == 2 && !m.violations);
    puts(
        "F4_PORT_PASS:reset mapping rejection,PA2/PA3 AF7,SR then DR error handling,TC finalization,disabled cache reset");
    return 0;
}
