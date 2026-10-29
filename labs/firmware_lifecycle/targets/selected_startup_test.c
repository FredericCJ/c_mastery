#include "selected_startup.h"
#include <stdio.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "ST_CHECK:%d:%s\n", __LINE__, #x); \
            return __LINE__;                                   \
        }                                                      \
    } while (0)

struct f4_model
{
    uint32_t a[64], v[64];
    unsigned n, w, s, bad, release;
    uint32_t generation, sequence, base;
};

static uint32_t rd(void *p, uint32_t a)
{
    struct f4_model *m = p;
    for (unsigned i = 0; i < m->n; ++i)
        if (m->a[i] == a)
            return m->v[i];
    return 0;
}

static void set(struct f4_model *m, uint32_t a, uint32_t v)
{
    for (unsigned i = 0; i < m->n; ++i)
        if (m->a[i] == a)
        {
            m->v[i] = v;
            return;
        }
    m->a[m->n] = a;
    m->v[m->n++] = v;
}

static void wr(void *p, uint32_t a, uint32_t v)
{
    struct f4_model *m = p;
    ++m->w;
    if (a == 0x580244A0u && (v & 8u))
    {
        ++m->release;
        if (!m->s || rd(m, 0x30040000) != m->generation ||
            rd(m, 0x30040020) != m->sequence ||
            rd(m, 0x30040024) != m->generation ||
            rd(m, 0x30040028) != m->base || rd(m, 0x3004002C) != ~m->sequence)
            m->bad = 1;
    }
    set(m, a, v);
}

static void sync(void *p)
{
    ++((struct f4_model *)p)->s;
}

static struct f4_model fresh(void)
{
    struct f4_model m = {0};
    set(&m, 0x58024400u, 5);
    set(&m, 0x58000700u, 1);
    set(&m, 0x52002040u, 0x0800);
    set(&m, 0x52002048u, 0x0810);
    return m;
}

int main(void)
{
    struct f4_model m = fresh();
    struct fh_io x = {&m, rd, wr, sync};
    C(st_cold_h745(x));
    C((rd(&m, 0x580244DC) & 0xA0000000u) == 0xA0000000u);
    C(rd(&m, 0x580244E0) & (1u << 25));
    const uint32_t addr[] = {0x58024400, 0x58024400, 0x58024400, 0x58024410,
                             0x58024418, 0x5802441C, 0x58024420, 0xE000ED14,
                             0xE000ED94, 0x580244A0, 0x58000704, 0x58000700,
                             0x52002040, 0x52002048};
    const uint32_t value[] = {1,       13, 0x01000005, 8, 8, 0x40,   0x40,
                              0x10000, 1,  8,          1, 0, 0x0801, 0x0800};
    for (unsigned i = 0; i < sizeof addr / sizeof *addr; ++i)
    {
        m = fresh();
        set(&m, addr[i], value[i]);
        C(!st_cold_h745(x));
        C(!m.release && !(rd(&m, 0x580244DC) & 0xA0000000u));
    }
    struct bs_selection s = {0};
    s.sequence = 9;
    s.image[0].core = 0;
    s.image[1].core = 1;
    s.image[0].generation = s.image[1].generation = 7;
    s.image[1].base = 0x08060200;
    m = fresh();
    C(st_cold_h745(x));
    m.s = 0;
    m.generation = 7;
    m.sequence = 9;
    m.base = s.image[1].base;
    C(st_release(x, &s) && !m.bad && m.release == 1);
    C(!st_release(x, &s) && m.release == 1);
    C(st_peer_matches(x, &s));
    for (unsigned i = 0; i < 4; ++i)
    {
        uint32_t a = 0x30040020 + 4 * i, v = rd(&m, a);
        set(&m, a, v ^ 1);
        C(!st_peer_matches(x, &s));
        set(&m, a, v);
    }
    m = fresh();
    s.image[1].generation = 8;
    C(!st_release(x, &s) && !m.w);
    s.image[1].generation = 7;
    s.sequence = 0;
    C(!st_release(x, &s) && !m.w);
    m = (struct f4_model){0};
    set(&m, 0x40023800, 3);
    C(st_cold_f439(x));
    set(&m, 0x40023808, 4);
    C(!st_cold_f439(x));
    set(&m, 0x40023808, 0);
    set(&m, 0x40023800, 0x01000003);
    C(!st_cold_f439(x));
    C(!st_cold_h745((struct fh_io){0}));
    puts(
        "ST_PASS:cold-clock,held-peer,bootstrap,publication-before-release,stale-mailbox");
    return 0;
}
