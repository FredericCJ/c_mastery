#include "h745_flash.h"
#ifndef HF_MUTANT
#define HF_MUTANT 0
#endif
#define CR 0x5200200Cu
#define SR 0x52002010u
#define CCR 0x52002014u
#define KEY 0x52002004u
#define ERR 0x07EE0000u
#define EOP 0x00010000u

enum fh_result fh_start(struct fh_session *s, struct fh_io io, uint32_t a,
                        const uint32_t w[8])
{
    if (!s || !w || !io.read || !io.write || !io.sync || s->active ||
        (a & 31u) || a < 0x08020000u || a > 0x080FFFE0u)
        return FH_REJECT;
    uint32_t sr = io.read(io.ctx, SR), cr = io.read(io.ctx, CR);
    if ((sr & 15u) || (cr & ~0x31u))
        return FH_REJECT;
    /* Source is immutable SRAM and does not overlap the session. */
    for (unsigned i = 0; i < 8; ++i)
        s->expected[i] = w[i];
    s->io = io;
    s->address = a;
    s->control = cr;
    s->active = 1;
    if ((cr & 1u) || HF_MUTANT == 1)
    {
        io.write(io.ctx, KEY, 0x45670123u);
        io.write(io.ctx, KEY, 0xCDEF89ABu);
        if (io.read(io.ctx, CR) & 1u)
        {
            s->active = 0;
            return FH_ERROR;
        }
    }
    io.write(io.ctx, CCR, ERR | EOP);
    io.write(io.ctx, CR, (cr & ~1u) | 2u);
    for (unsigned i = 0; i < 8; ++i)
        io.write(io.ctx, a + 4u * i, s->expected[i]);
    io.sync(io.ctx);
    return FH_PENDING;
}

enum fh_result fh_poll(struct fh_session *s)
{
    if (!s || s->active != 1u)
        return FH_REJECT;
    struct fh_io io = s->io;
    uint32_t sr = io.read(io.ctx, SR);
    if (HF_MUTANT != 2 && (sr & 7u))
        return FH_PENDING;
    /* A cleared old EOP followed by a new EOP witnesses fast completion even
       if polling did not sample QW high. No software timeout cancels hardware. */
    if (!(sr & (EOP | ERR)))
        return FH_PENDING;
    io.sync(io.ctx);
    int good = !(sr & ERR);
    if (good)
        for (unsigned i = 0; i < 8; ++i)
            if (io.read(io.ctx, s->address + 4u * i) != s->expected[i])
                good = 0;
    io.write(io.ctx, CR, s->control & ~1u); /* PG off only after quiescence. */
    if (s->control & 1u)
        io.write(io.ctx, CR, s->control);
    io.sync(io.ctx);
    s->active = 0;
    return good ? FH_DONE : FH_ERROR;
}

static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (8 * i));
}

static uint32_t crc(const unsigned char *p, unsigned n)
{
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < n; ++i)
    {
        c ^= p[i];
        for (unsigned b = 0; b < 8; ++b)
            c = (c >> 1) ^ ((0u - (c & 1u)) & 0xEDB88320u);
    }
    return ~c;
}

int fh_record(const struct fw_peer *p, uint32_t n, const unsigned char h[64],
              const uint32_t v[2], const uint32_t ver[2],
              unsigned char out[128])
{
    if (!p || !h || !v || !ver || !out || !n || !p->generation ||
        p->health != 3u || p->failed || p->confirmed != 1u ||
        p->outstanding[0] || p->outstanding[1] || p->reserved[0] ||
        p->reserved[1])
        return 0;
    for (unsigned i = 0; i < 128; ++i)
        out[i] = 0;
    put(out, 0x314A3642u);
    put(out + 4, n);
    put(out + 8, p->generation);
    for (unsigned i = 0; i < 64; ++i)
        out[16 + i] = h[i];
    for (unsigned i = 0; i < 2; ++i)
    {
        put(out + 80 + 4 * i, v[i]);
        put(out + 88 + 4 * i, ver[i]);
    }
    put(out + 96, 0x31433642u);
    put(out + 100, n);
    put(out + 104, p->generation);
    put(out + 108, ~n);
    put(out + 112, crc(out, 96));
    return 1;
}

int fh_record_valid(const unsigned char in[128])
{
    if (!in || get(in) != 0x314A3642u || !get(in + 4) || !get(in + 8) ||
        get(in + 12))
        return 0;
    if (HF_MUTANT != 3 &&
        (get(in + 96) != 0x31433642u || get(in + 100) != get(in + 4) ||
         get(in + 104) != get(in + 8) || get(in + 108) != ~get(in + 4) ||
         get(in + 112) != crc(in, 96)))
        return 0;
    if (HF_MUTANT != 3)
        for (unsigned i = 116; i < 128; ++i)
            if (in[i])
                return 0;
    return 1;
}

static enum fh_result next_word(struct fh_append *a)
{
    uint32_t words[8];
    for (unsigned i = 0; i < 8; ++i)
        words[i] = get(a->record + 32 * a->word + 4 * i);
    enum fh_result r =
        fh_start(&a->session, a->io, a->base + 32 * a->word, words);
    if (r != FH_PENDING)
        a->state = 3;
    return r;
}

enum fh_result fh_append_begin(struct fh_append *a, struct fh_io io,
                               uint32_t base, const struct fw_peer *p,
                               uint32_t n, const unsigned char h[64],
                               const uint32_t v[2], const uint32_t ver[2])
{
    if (!a || a->state || a->session.active || !io.read || !io.write ||
        !io.sync || (base & 31u) || base < 0x08020000u || base > 0x080FFF80u)
        return FH_REJECT;
    if (!fh_record(p, n, h, v, ver, a->record))
        return FH_REJECT;
    a->io = io;
    a->base = base;
    a->word = 0;
    a->state = 1;
    return next_word(a);
}

enum fh_result fh_append_poll(struct fh_append *a)
{
    if (!a || a->state != 1u)
        return FH_REJECT;
    enum fh_result r = fh_poll(&a->session);
    if (r == FH_PENDING)
        return r;
    if (r != FH_DONE)
    {
        a->state = 3;
        return r;
    }
    if (++a->word == 4u)
    {
        a->state = 2;
        return FH_DONE;
    }
    return next_word(a);
}
