#include "trial_intent.h"
#ifndef FT_MUTANT
#define FT_MUTANT 0
#endif
static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put(unsigned char *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(n >> (8 * i));
}

static uint32_t crc(const unsigned char *p, unsigned n)
{
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < n; ++i)
    {
        c ^= p[i];
        for (unsigned k = 0; k < 8; ++k)
            c = (c >> 1) ^ ((0u - (c & 1u)) & 0xEDB88320u);
    }
    return ~c;
}

static int valid(const unsigned char *b)
{
    if (get(b) != 0x31495446u || !get(b + 4) || get(b + 4) == UINT32_MAX ||
        get(b + 8) != get(b + 4) + 1 || !get(b + 12) ||
        get(b + 96) != 0x31435446u || get(b + 100) != get(b + 8) ||
        get(b + 104) != get(b + 12) || get(b + 108) != ~get(b + 8) ||
        get(b + 112) != crc(b, 96))
        return 0;
    for (unsigned i = 116; i < 128; ++i)
        if (b[i])
            return 0;
    return 1;
}

int ft_intent(const struct bs_selection *o, const struct fb_image in[2],
              unsigned char out[128])
{
    if (!o || !in || !out || !o->sequence || o->sequence == UINT32_MAX ||
        o->record > 1 || o->image[0].core != 0 || o->image[1].core != 1 ||
        !o->image[0].generation || in[0].core != 0 || in[1].core != 1 ||
        !in[0].generation || in[0].generation != in[1].generation ||
        o->image[0].generation != o->image[1].generation ||
        in[0].generation <= o->image[0].generation)
        return 0;
    for (unsigned c = 0; c < 2; ++c)
        if (in[c].base == o->image[c].base ||
            in[c].version < o->image[c].version)
            return 0;
    unsigned char b[128];
    for (unsigned i = 0; i < 128; ++i)
        b[i] = 0;
    put(b, 0x31495446u);
    put(b + 4, o->sequence);
    put(b + 8, o->sequence + 1);
    put(b + 12, in[0].generation);
    for (unsigned c = 0; c < 2; ++c)
    {
        for (unsigned i = 0; i < 32; ++i)
            b[16 + 32 * c + i] = in[c].digest[i];
        put(b + 80 + 4 * c, in[c].base);
        put(b + 88 + 4 * c, in[c].version);
    }
    put(b + 96, 0x31435446u);
    put(b + 100, o->sequence + 1);
    put(b + 104, in[0].generation);
    put(b + 108, ~(o->sequence + 1));
    put(b + 112, crc(b, 96));
    for (unsigned i = 0; i < 128; ++i)
        out[i] = b[i];
    return 1;
}

int ft_attempt(const unsigned char b[128], unsigned char out[32])
{
    if (!b || !out || !valid(b))
        return 0;
    unsigned char a[32];
    for (unsigned i = 0; i < 32; ++i)
        a[i] = 0;
    put(a, 0x31415446u);
    put(a + 4, get(b + 8));
    put(a + 8, get(b + 12));
    put(a + 12, ~get(b + 8));
    put(a + 16, crc(a, 16));
    for (unsigned i = 0; i < 32; ++i)
        out[i] = a[i];
    return 1;
}

int ft_probe(eg_read_fn read, void *ctx, const struct fb_policy *p,
             const struct bs_layouts *l, const struct bs_selection *o,
             struct bs_selection *out)
{
    if (!read || !p || !l || !o || !out || !o->sequence || o->record > 1)
        return 0;
    unsigned char b[160];
    for (unsigned i = 0; i < 40; ++i)
    {
        uint32_t n;
        if (read(ctx, 0x080A0000u + 4 * i, &n) != 1)
            return 0;
        put(b + 4 * i, n);
    }
    if (!valid(b) || (FT_MUTANT != 1 && get(b + 4) != o->sequence))
        return 0;
    unsigned char expected[32];
    if (!ft_attempt(b, expected))
        return 0;
    if (FT_MUTANT != 2)
        for (unsigned i = 0; i < 32; ++i)
            if (b[128 + i] != expected[i])
                return 0;
    struct bs_selection s;
    s.sequence = get(b + 8);
    s.record = o->record;
    for (unsigned c = 0; c < 2; ++c)
    {
        uint32_t base = get(b + 80 + 4 * c);
        unsigned slot = 2;
        for (unsigned n = 0; n < 2; ++n)
            if (l->core[c][n].base == base)
            {
                if (slot != 2)
                    return 0;
                slot = n;
            }
        if (slot == 2 || base == o->image[c].base ||
            !bs_read_image(read, ctx, p, l->core[c][slot], c, &s.image[c]) ||
            s.image[c].version != get(b + 88 + 4 * c) ||
            s.image[c].generation != get(b + 12) ||
            s.image[c].generation <= o->image[c].generation ||
            s.image[c].version < o->image[c].version)
            return 0;
        for (unsigned i = 0; i < 32; ++i)
            if (s.image[c].digest[i] != b[16 + 32 * c + i])
                return 0;
    }
    out->sequence = s.sequence;
    out->record = s.record;
    for (unsigned c = 0; c < 2; ++c)
    {
        struct fb_image *d = &out->image[c];
        const struct fb_image *v = &s.image[c];
        d->version = v->version;
        d->generation = v->generation;
        d->base = v->base;
        d->stack = v->stack;
        d->entry = v->entry;
        d->extent = v->extent;
        d->key = v->key;
        d->core = v->core;
        for (unsigned i = 0; i < 32; ++i)
            d->digest[i] = v->digest[i];
    }
    return 1;
}

#define CR 0x5200200Cu
#define SR 0x52002010u
#define CCR 0x52002014u
#define ERR 0x07EE0000u

static enum fh_result program(struct ft_writer *w)
{
    uint32_t values[8];
    for (unsigned i = 0; i < 8; ++i)
        values[i] = get(w->bytes + w->word * 32 + 4 * i);
    enum fh_result z =
        fh_start(&w->program, w->io.bus, 0x080A0000u + w->word * 32, values);
    if (z != FH_PENDING)
        w->state = 6;
    return z;
}

enum fh_result ft_begin(struct ft_writer *w, struct fm_io io,
                        const struct bs_selection *old,
                        const struct fb_image installed[2])
{
    if (!w || w->state || !io.bus.read || !io.bus.write || !io.bus.sync ||
        !io.fresh)
        return FH_REJECT;
    unsigned char b[160];
    if (!ft_intent(old, installed, b) || !ft_attempt(b, b + 128))
        return FH_REJECT;
    uint32_t cr = io.bus.read(io.bus.ctx, CR), sr = io.bus.read(io.bus.ctx, SR);
    if ((sr & 15u) || (cr & ~0x31u))
        return FH_REJECT;
    w->io = io;
    w->control = cr;
    w->checked = 0;
    w->word = 0;
    w->program.active = 0;
    for (unsigned i = 0; i < 160; ++i)
        w->bytes[i] = b[i];
    if (cr & 1u)
    {
        io.bus.write(io.bus.ctx, 0x52002004u, 0x45670123u);
        io.bus.write(io.bus.ctx, 0x52002004u, 0xCDEF89ABu);
        if (io.bus.read(io.bus.ctx, CR) & 1u)
        {
            w->state = 6;
            return FH_ERROR;
        }
    }
    io.bus.write(io.bus.ctx, CCR, ERR | 0x10000u);
    uint32_t active = (cr & 0x30u) | 4u | (5u << 8);
    io.bus.write(io.bus.ctx, CR, active);
    io.bus.write(io.bus.ctx, CR, active | 0x80u);
    io.bus.sync(io.bus.ctx);
    w->state = 1;
    return FH_PENDING;
}

enum fh_result ft_poll(struct ft_writer *w)
{
    if (!w || w->state < 1 || w->state > 3)
        return FH_REJECT;
    struct fh_io io = w->io.bus;
    if (w->state == 1)
    {
        uint32_t sr = io.read(io.ctx, SR);
        if (sr & 7u)
            return FH_PENDING;
        if (!(sr & (ERR | 0x10000u)))
            return FH_PENDING;
        io.sync(io.ctx);
        io.write(io.ctx, CR, w->control & ~1u);
        if (w->control & 1u)
            io.write(io.ctx, CR, w->control);
        io.sync(io.ctx);
        if (sr & ERR)
        {
            w->state = 6;
            return FH_ERROR;
        }
        w->io.fresh(io.ctx, 0x080A0000u, 0x20000);
        w->state = 2;
    }
    if (w->state == 2)
    {
        for (unsigned n = 0; n < 32 && w->checked < 0x20000u;
             ++n, w->checked += 4)
            if (io.read(io.ctx, 0x080A0000u + w->checked) != UINT32_MAX)
            {
                w->state = 6;
                return FH_ERROR;
            }
        if (io.read(io.ctx, SR) & ERR)
        {
            w->state = 6;
            return FH_ERROR;
        }
        if (w->checked < 0x20000u)
            return FH_PENDING;
        w->state = 3;
        return program(w);
    }
    enum fh_result z = fh_poll(&w->program);
    if (z == FH_PENDING)
        return z;
    if (z != FH_DONE)
    {
        w->state = 6;
        return z;
    }
    if (++w->word < 5)
        return program(w);
    w->io.fresh(io.ctx, 0x080A0000u, 160);
    for (unsigned i = 0; i < 40; ++i)
        if (io.read(io.ctx, 0x080A0000u + 4 * i) != get(w->bytes + 4 * i))
        {
            w->state = 6;
            return FH_ERROR;
        }
    if (io.read(io.ctx, SR) & ERR)
    {
        w->state = 6;
        return FH_ERROR;
    }
    w->state = 4;
    return FH_DONE;
}
