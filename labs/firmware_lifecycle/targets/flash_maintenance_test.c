#include "flash_maintenance.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "FM_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct f4_model
{
    uint32_t mem[65536], cr, sr, pending_base, words[8], address[8];
    unsigned h, key, busy, erase, count, fresh, writes, violations, error,
        corrupt, fail_verify;
    unsigned cut, step, dead;
};
static struct f4_model m;

static uint32_t rd(void *v, uint32_t a)
{
    struct f4_model *s = v;
    if (a == (s->h ? 0x5200200Cu : 0x40023C10u))
        return s->cr;
    if (a == (s->h ? 0x52002010u : 0x40023C0Cu))
        return s->sr;
    if (a >= 0x080C0000u && a < 0x08100000u)
    {
        if (s->busy)
            s->violations++;
        return s->mem[(a - 0x080C0000u) / 4];
    }
    s->violations++;
    return 0;
}

static void wr(void *v, uint32_t a, uint32_t n)
{
    struct f4_model *s = v;
    if (s->dead)
        return;
    s->writes++;
    uint32_t cr = s->h ? 0x5200200Cu : 0x40023C10u,
             key = s->h ? 0x52002004u : 0x40023C04u,
             clear = s->h ? 0x52002014u : 0x40023C0Cu,
             lock = s->h ? 1u : 0x80000000u;
    if (a == key)
    {
        if (!s->key && n == 0x45670123u)
            s->key = 1;
        else if (s->key == 1 && n == 0xCDEF89ABu)
        {
            s->cr &= ~lock;
            s->key = 0;
        }
        else
            s->violations++;
        return;
    }
    if (a == clear)
    {
        s->sr &= ~n;
        return;
    }
    if (a == cr)
    {
        if (s->busy)
        {
            s->violations++;
            return;
        }
        s->cr = n;
        if (n & (s->h ? 0x80u : 0x10000u))
        {
            unsigned sec = s->h ? (n >> 8) & 7u : (n >> 3) & 31u;
            s->pending_base = s->h ? 0x08000000u + sec * 0x20000u
                                   : 0x080C0000u + (sec - 10) * 0x20000u;
            s->erase = 1;
            s->busy = 1;
            s->sr = s->h ? 5u : 0x10000u;
            if (n & (s->h ? 8u : 0x8004u))
                s->violations++;
        }
        return;
    }
    if (a >= 0x080C0000u && a < 0x08100000u)
    {
        if (!(s->cr & (s->h ? 2u : 1u)) || s->count >= 8)
            s->violations++;
        else
        {
            s->address[s->count] = a;
            s->words[s->count++] = n;
            s->busy = 1;
            s->sr = s->h ? 5u : 0x10000u;
        }
        return;
    }
    s->violations++;
}

static void sync_io(void *v)
{
    (void)v;
}

static void fresh(void *v, uint32_t a, uint32_t n)
{
    struct f4_model *s = v;
    (void)a;
    (void)n;
    if (s->busy)
        s->violations++;
    s->fresh++;
    if (s->fail_verify && s->fresh == 3)
        s->mem[0x20000u / 4 + 29] ^= 1u;
}

static struct fm_io io(void)
{
    return (struct fm_io){{&m, rd, wr, sync_io}, fresh};
}

static void init(unsigned h)
{
    memset(&m, 0, sizeof m);
    m.h = h;
    m.cr = h ? 0x31u : 0x80000300u;
    for (unsigned i = 0; i < 65536; ++i)
        m.mem[i] = UINT32_MAX;
}

static int cut(void)
{
    if (++m.step == m.cut)
    {
        m.dead = 1;
        return 1;
    }
    return 0;
}

static void finish(void)
{
    if (!m.busy || m.dead)
        return;
    if (!m.error)
    {
        if (m.erase)
        {
            unsigned start = (m.pending_base - 0x080C0000u) / 4;
            for (unsigned i = 0; i < 32768; i += 8)
            {
                for (unsigned j = 0; j < 8; ++j)
                    m.mem[start + i + j] = UINT32_MAX;
                if (cut())
                    return;
            }
        }
        else
        {
            for (unsigned i = 0; i < m.count; ++i)
            {
                m.mem[(m.address[i] - 0x080C0000u) / 4] &= m.words[i];
                if (cut())
                    return;
            }
        }
    }
    m.busy = 0;
    m.erase = 0;
    m.count = 0;
    m.cr &= ~(m.h ? 0x80u : 0x10000u);
    m.sr = m.error ? (m.h ? 0x20000u : 0x10u) : (m.h ? 0x10000u : 0u);
    if (m.corrupt)
        m.mem[(m.pending_base - 0x080C0000u) / 4] = 0;
}

static enum fh_result drain(struct fm_operation *o)
{
    enum fh_result r = FH_PENDING;
    for (unsigned i = 0; i < 1100 && r == FH_PENDING; ++i)
    {
        finish();
        r = fm_poll(o);
    }
    return r;
}

static void bytes(uint32_t base, unsigned char out[128])
{
    for (unsigned i = 0; i < 128; ++i)
        out[i] = (unsigned char)(m.mem[(base - 0x080C0000u + i) / 4] >>
                                 (8 * (i % 4)));
}

static void store(uint32_t base, const unsigned char in[128])
{
    for (unsigned i = 0; i < 32; ++i)
        m.mem[(base - 0x080C0000u) / 4 + i] =
            (uint32_t)in[4 * i] | (uint32_t)in[4 * i + 1] << 8 |
            (uint32_t)in[4 * i + 2] << 16 | (uint32_t)in[4 * i + 3] << 24;
}

static struct fw_peer peer(void)
{
    struct fw_peer p = {0};
    p.generation = 7;
    p.health = 3;
    p.confirmed = 1;
    return p;
}

int main(void)
{
    for (unsigned profile = 0; profile < 2; ++profile)
    {
        init(profile);
        struct fm_operation o = {0};
        enum fm_profile p = profile ? FM_H745 : FM_F439;
        unsigned sector = profile ? 6 : 10;
        m.mem[0] = 0;
        C(fm_erase(&o, io(), p, sector - 1) == FH_REJECT && !m.writes);
        C(fm_erase(&o, io(), p, sector) == FH_PENDING);
        unsigned w = m.writes;
        C(fm_poll(&o) == FH_PENDING && m.writes == w && !m.fresh &&
          !m.violations);
        C(drain(&o) == FH_DONE && o.checked == 0x20000u &&
          m.mem[0] == UINT32_MAX && m.fresh == 1 && !m.violations);
        C(m.cr == (profile ? 0x31u : 0x80000300u));
        C(fm_poll(&o) == FH_REJECT);
        init(profile);
        o = (struct fm_operation){0};
        m.error = 1;
        C(fm_erase(&o, io(), p, sector) == FH_PENDING);
        C(drain(&o) == FH_ERROR && !m.fresh && !m.violations);
        init(profile);
        o = (struct fm_operation){0};
        m.corrupt = 1;
        C(fm_erase(&o, io(), p, sector) == FH_PENDING);
        C(drain(&o) == FH_ERROR && !m.violations);
    }
    init(0);
    struct fm_operation f = {0};
    C(fm_f439_program(&f, io(), 0x080C0001u, 17) == FH_REJECT);
    C(fm_f439_program(&f, io(), 0x080C0000u, 0x11223344u) == FH_PENDING);
    C(drain(&f) == FH_DONE && m.mem[0] == 0x11223344u &&
      !m.violations); /* No EOPIE: EOP stays0. */
    f = (struct fm_operation){0};
    C(fm_f439_program(&f, io(), 0x080C0000u, UINT32_MAX) == FH_PENDING);
    C(drain(&f) == FH_ERROR); /* Cannot set a programmed zero to1. */
    struct fw_peer p = peer();
    unsigned char hashes[64] = {0}, old[128], a[128], b[128];
    uint32_t vectors[2] = {0x08040000u, 0x08140000u}, versions[2] = {1, 2};
    C(fh_record(&p, 1, hashes, vectors, versions, old));
    /* Every32-bit record-store cut plus first/middle/last erase granules. */
    unsigned cuts[40] = {1, 2, 2048, 4096};
    unsigned nc = 4;
    for (unsigned i = 1; i <= 32; ++i)
        cuts[nc++] = 4096 + i;
    cuts[nc++] = 4096 + 32 + 1;
    cuts[nc++] = 4096 + 32 + 2048;
    cuts[nc++] = 8192 + 32;
    cuts[nc++] = 0;
    for (unsigned c = 0; c < nc; ++c)
    {
        init(1);
        store(0x080C0000u, old);
        m.cut = cuts[c];
        struct fm_recycle r = {0};
        C(fm_recycle_begin(&r, io(), 0x080C0000u, &p, 2, hashes, vectors,
                           versions) == FH_PENDING);
        enum fh_result e = FH_PENDING;
        for (unsigned i = 0; i < 2200 && e == FH_PENDING && !m.dead; ++i)
        {
            finish();
            if (!m.dead)
                e = fm_recycle_poll(&r);
        }
        C(m.dead || e == FH_DONE);
        C(!m.violations);
        bytes(0x080C0000u, a);
        bytes(0x080E0000u, b);
        int recovered = fm_recover(a, b);
        C(recovered == 1 || recovered == 2);
        if (!m.cut)
            C(recovered == 2 && r.state == 4);
        if (m.cut && m.cut <= 4096 + 24)
            C(recovered == 1);
    }
    init(1);
    store(0x080C0000u, old);
    m.fail_verify = 1;
    struct fm_recycle r = {0};
    C(fm_recycle_begin(&r, io(), 0x080C0000u, &p, 2, hashes, vectors,
                       versions) == FH_PENDING);
    enum fh_result e = FH_PENDING;
    for (unsigned i = 0; i < 2200 && e == FH_PENDING; ++i)
    {
        finish();
        e = fm_recycle_poll(&r);
    }
    C(e == FH_ERROR && r.state == 5);
    bytes(0x080C0000u, a);
    C(fh_record_valid(a));
    memcpy(b, old, 128);
    C(fm_recover(old, b) == 1);
    b[20] ^= 1;
    C(fm_recover(old, b) == 1);
    C(fh_record(&p, 1, hashes, vectors, (uint32_t[2]){2, 3}, b));
    C(fm_recover(old, b) == -1);
    printf(
        "FM_PASS:2profiles,40digitalcut histories,32record-store cuts,retained-old recovery\n");
    return 0;
}
