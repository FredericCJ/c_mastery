#include "inactive_install.h"
#include "monocypher-ed25519.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "FU_CHECK:%d:%s\n", __LINE__, #x); \
            return __LINE__;                                   \
        }                                                      \
    } while (0)

struct f4_model
{
    uint32_t mem[262144], cr, sr, base, words[8], address[8];
    unsigned key, busy, erase, count, fresh, violations, cut, step, dead,
        erasecut, error, corrupt;
};
static struct f4_model m;
static struct rt_frame incoming;
static struct fb_policy policy;
static struct bs_layouts layout = {
    .core = {{{0x08020200, 0x08022000, 0x20000000, 0x20020000},
              {0x08040200, 0x08042000, 0x20000000, 0x20020000}},
             {{0x08060200, 0x08062000, 0x30000000, 0x30020000},
              {0x08080200, 0x08082000, 0x30000000, 0x30020000}}}};

static uint32_t rd(void *c, uint32_t a)
{
    struct f4_model *s = c;
    if (a == 0x5200200C)
        return s->cr;
    if (a == 0x52002010)
        return s->sr;
    if (a >= 0x08000000 && a < 0x08100000)
    {
        if (s->busy)
            ++s->violations;
        return s->mem[(a - 0x08000000) / 4];
    }
    ++s->violations;
    return 0;
}

static int read_slot(void *c, uint32_t a, uint32_t *out)
{
    if ((a & 3) || a < 0x08000000 || a >= 0x08100000)
        return 0;
    *out = rd(c, a);
    return 1;
}

static void wr(void *c, uint32_t a, uint32_t n)
{
    struct f4_model *s = c;
    if (s->dead)
        return;
    if (a == 0x52002004)
    {
        if (!s->key && n == 0x45670123)
            s->key = 1;
        else if (s->key == 1 && n == 0xCDEF89AB)
        {
            s->cr &= ~1u;
            s->key = 0;
        }
        else
            ++s->violations;
        return;
    }
    if (a == 0x52002014)
    {
        s->sr &= ~n;
        return;
    }
    if (a == 0x5200200C)
    {
        if (s->busy)
        {
            ++s->violations;
            return;
        }
        s->cr = n;
        if (n & 0x80)
        {
            s->base = 0x08000000 + ((n >> 8) & 7) * 0x20000;
            if (s->base != 0x08040000)
                ++s->violations;
            s->erase = 1;
            s->busy = 1;
            s->sr = 5;
        }
        return;
    }
    if (a >= 0x08040000 && a < 0x08060000)
    {
        if (!(s->cr & 2) || s->count >= 8)
            ++s->violations;
        else
        {
            s->address[s->count] = a;
            s->words[s->count++] = n;
            s->busy = 1;
            s->sr = 5;
        }
        return;
    }
    ++s->violations;
}

static void sync_io(void *c)
{
    (void)c;
}

static void fresh(void *c, uint32_t a, uint32_t n)
{
    struct f4_model *s = c;
    if (s->busy || a < 0x08040000 || a + n > 0x08060000)
        ++s->violations;
    ++s->fresh;
    if (s->corrupt && s->fresh == 2)
        s->mem[(0x08040200 - 0x08000000) / 4 + 129] ^= 1;
}

static struct fm_io io(void)
{
    return (struct fm_io){{&m, rd, wr, sync_io}, fresh};
}

static void finish(void)
{
    if (!m.busy || m.dead)
        return;
    if (!m.error)
    {
        if (m.erase)
        {
            unsigned start = (m.base - 0x08000000) / 4;
            for (unsigned i = 0; i < 32768; i += 8)
            {
                for (unsigned j = 0; j < 8; ++j)
                    m.mem[start + i + j] = UINT32_MAX;
                if (m.erasecut == i / 8 + 1)
                {
                    m.dead = 1;
                    return;
                }
            }
        }
        else
            for (unsigned i = 0; i < m.count; ++i)
            {
                m.mem[(m.address[i] - 0x08000000) / 4] &= m.words[i];
                if (++m.step == m.cut)
                {
                    m.dead = 1;
                    return;
                }
            }
    }
    m.busy = m.erase = m.count = 0;
    m.cr &= ~0x80u;
    m.sr = m.error ? 0x20000u : 0x10000u;
}

static void put(unsigned char *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(n >> (8 * i));
}

static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void make(struct rt_frame *f, unsigned core, unsigned slot,
                 unsigned generation)
{
    unsigned char packet[684] = {0};
    packet[0] = 'B';
    packet[1] = '6';
    packet[2] = 'U';
    packet[3] = 'P';
    packet[4] = 1;
    packet[5] = (unsigned char)core;
    packet[6] = (unsigned char)slot;
    packet[8] = 0x60;
    packet[9] = 2;
    unsigned char *e = packet + 12;
    put(e, 0x31423646);
    e[4] = 1;
    e[5] = (unsigned char)core;
    put(e + 8, generation);
    put(e + 12, generation);
    put(e + 16, layout.core[core][slot].base);
    put(e + 20, 576);
    put(e + 32, layout.core[core][slot].stack_high);
    put(e + 36, layout.core[core][slot].base + 513);
    unsigned char seed[32] = {0}, secret[64];
    crypto_ed25519_key_pair(secret, policy.keys[0], seed);
    crypto_ed25519_sign(e + 608, secret, e, 608);
    rt_reset(f);
    for (unsigned i = 0; i < 684; ++i)
        (void)rt_byte(f, packet[i]);
}

static void store(uint32_t a, const unsigned char *p, unsigned n)
{
    for (unsigned i = 0; i < n; i += 4)
        m.mem[(a - 0x08000000 + i) / 4] = get(p + i);
}

static void init(const unsigned char *record, const struct rt_frame old[2])
{
    memset(&m, 0, sizeof m);
    m.cr = 0x31;
    for (unsigned i = 0; i < 262144; ++i)
        m.mem[i] = UINT32_MAX;
    for (unsigned i = 0; i < 32768; ++i)
        m.mem[0x40000 / 4 + i] = 0;
    for (unsigned c = 0; c < 2; ++c)
    {
        uint32_t base = layout.core[c][0].base;
        store(base - 32, old[c].bytes + 12, 608);
        store(base + 2048, old[c].bytes + 620, 64);
    }
    store(0x080C0000, record, 128);
}

static enum fh_result drain(struct fu_install *f, struct fb_image *out)
{
    enum fh_result r = FH_PENDING;
    for (unsigned i = 0; i < 1200 && r == FH_PENDING && !m.dead; ++i)
    {
        finish();
        if (!m.dead)
            r = fu_poll(f, out);
    }
    return r;
}

int main(void)
{
    policy.present = 2;
    policy.floor[0] = policy.floor[1] = policy.generation_floor = 1;
    struct rt_frame prior[2];
    struct fb_image previous[2];
    for (unsigned c = 0; c < 2; ++c)
    {
        make(&prior[c], c, 0, 1);
        C(rt_admit(&prior[c], &policy, &layout, &previous[c]));
    }
    struct fw_peer peer = {.generation = 1, .health = 3, .confirmed = 1};
    unsigned char record[128];
    C(fb_joint_record(previous, &peer, 1, record));
    struct bs_selection old = {.sequence = 1, .record = 0};
    old.image[0] = previous[0];
    old.image[1] = previous[1];
    make(&incoming, 0, 1, 2);
    struct fb_image out, before;
    memset(&before, 0xA5, sizeof before);
    for (unsigned cut = 0; cut <= 536; ++cut)
    {
        init(record, prior);
        m.cut = cut;
        struct fu_install f = {0};
        out = before;
        C(fu_begin(&f, io(), read_slot, &m, &incoming, &policy, &layout,
                   &old) == FH_PENDING);
        C(fu_poll(&f, &out) == FH_PENDING &&
          !memcmp(&out, &before, sizeof out));
        enum fh_result z = drain(&f, &out);
        C(!m.violations);
        C(!memcmp((unsigned char *)m.mem + 0xC0000, record, 128));
        for (unsigned c = 0; c < 2; ++c)
        {
            uint32_t off = layout.core[c][0].base - 0x08000000;
            C(!memcmp((unsigned char *)m.mem + off - 32, prior[c].bytes + 12,
                      608));
            C(!memcmp((unsigned char *)m.mem + off + 2048, prior[c].bytes + 620,
                      64));
        }
        if (cut)
            C(m.dead && !memcmp(&out, &before, sizeof out));
        else
            C(z == FH_DONE && f.state == 4 && out.generation == 2 &&
              out.base == 0x08040200 && m.step == 536 && m.fresh == 2);
    }
    const unsigned erasecuts[] = {1, 2048, 4096};
    for (unsigned i = 0; i < 3; ++i)
    {
        init(record, prior);
        m.erasecut = erasecuts[i];
        struct fu_install f = {0};
        out = before;
        C(fu_begin(&f, io(), read_slot, &m, &incoming, &policy, &layout,
                   &old) == FH_PENDING);
        (void)drain(&f, &out);
        C(m.dead && !memcmp(&out, &before, sizeof out) &&
          !memcmp((unsigned char *)m.mem + 0xC0000, record, 128));
    }
    init(record, prior);
    m.corrupt = 1;
    struct fu_install f = {0};
    out = before;
    C(fu_begin(&f, io(), read_slot, &m, &incoming, &policy, &layout, &old) ==
      FH_PENDING);
    C(drain(&f, &out) == FH_ERROR && !memcmp(&out, &before, sizeof out));
    init(record, prior);
    f = (struct fu_install){0};
    struct rt_frame active;
    make(&active, 0, 0, 2);
    C(fu_begin(&f, io(), read_slot, &m, &active, &policy, &layout, &old) ==
          FH_REJECT &&
      !m.busy);
    init(record, prior);
    m.error = 1;
    f = (struct fu_install){0};
    out = before;
    C(fu_begin(&f, io(), read_slot, &m, &incoming, &policy, &layout, &old) ==
      FH_PENDING);
    C(drain(&f, &out) == FH_ERROR && !m.violations &&
      !memcmp(&out, &before, sizeof out));
    puts(
        "FU_PASS:536program-store cuts,3erase cuts,old pair preserved,actual signature readback,active-slot refusal");
    return 0;
}
