#include "inactive_install.h"
#ifndef FU_MUTANT
#define FU_MUTANT 0
#endif
#define CR 0x5200200Cu
#define SR 0x52002010u
#define CCR 0x52002014u
#define ERR 0x07EE0000u

static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static enum fh_result word(struct fu_install *f)
{
    uint32_t w[8];
    for (unsigned i = 0; i < 8; ++i)
    {
        w[i] = get(f->bytes + 32 * f->word + 4 * i);
    }
    enum fh_result z =
        fh_start(&f->program, f->io.bus, f->layout.base - 32 + 32 * f->word, w);
    if (z != FH_PENDING)
    {
        f->state = 6;
    }
    return z;
}

enum fh_result fu_begin(struct fu_install *f, struct fm_io io, eg_read_fn read,
                        void *ctx, const struct rt_frame *t,
                        const struct fb_policy *p, const struct bs_layouts *l,
                        const struct bs_selection *old)
{
    if (!f || f->state || !io.bus.read || !io.bus.write || !io.bus.sync ||
        !io.fresh || !read || !old || !old->sequence || old->record > 1)
    {
        return FH_REJECT;
    }
    struct fb_image candidate;
    if (!rt_admit(t, p, l, &candidate))
    {
        return FH_REJECT;
    }
    unsigned c = candidate.core;
    if (old->image[0].core != 0 || old->image[1].core != 1 ||
        !old->image[0].generation ||
        old->image[0].generation != old->image[1].generation ||
        candidate.generation <= old->image[c].generation ||
        candidate.version < old->image[c].version ||
        (FU_MUTANT != 1 && candidate.base == old->image[c].base))
    {
        return FH_REJECT;
    }
    uint32_t base = candidate.base;
    if (base < 0x08020200u || base > 0x08080200u ||
        ((base - 0x08000200u) % 0x20000u))
    {
        return FH_REJECT;
    }
    unsigned sector = (base - 0x08000200u) / 0x20000u;
    if ((c == 0 && (sector < 1 || sector > 2)) ||
        (c == 1 && (sector < 3 || sector > 4)))
    {
        return FH_REJECT;
    }
    uint32_t cr = io.bus.read(io.bus.ctx, CR), sr = io.bus.read(io.bus.ctx, SR);
    if ((sr & 15u) || (cr & ~0x31u))
    {
        return FH_REJECT;
    }
    f->io = io;
    f->read = read;
    f->read_ctx = ctx;
    for (unsigned k = 0; k < 2; ++k)
    {
        for (unsigned i = 0; i < 32; ++i)
        {
            f->policy.keys[k][i] = p->keys[k][i];
        }
    }
    f->policy.present = p->present;
    f->policy.revoked = p->revoked;
    for (unsigned i = 0; i < 2; ++i)
    {
        f->policy.floor[i] = p->floor[i];
    }
    f->policy.generation_floor = p->generation_floor;
    f->layout = l->core[c][t->bytes[6]];
    f->expected = candidate;
    f->sector_base = 0x08000000u + sector * 0x20000u;
    f->control = cr;
    f->checked = 0;
    f->word = 0;
    f->program.active = 0;
    for (unsigned i = 0; i < 2144; ++i)
    {
        f->bytes[i] = 255;
    }
    unsigned n = (unsigned)t->bytes[8] | (unsigned)t->bytes[9] << 8;
    for (unsigned i = 0; i < n; ++i)
    {
        f->bytes[i] = t->bytes[12 + i];
    }
    for (unsigned i = 0; i < 64; ++i)
    {
        f->bytes[2080 + i] = t->bytes[12 + n + i];
    }
    if (cr & 1u)
    {
        io.bus.write(io.bus.ctx, 0x52002004u, 0x45670123u);
        io.bus.write(io.bus.ctx, 0x52002004u, 0xCDEF89ABu);
        if (io.bus.read(io.bus.ctx, CR) & 1u)
        {
            f->state = 6;
            return FH_ERROR;
        }
    }
    io.bus.write(io.bus.ctx, CCR, ERR | 0x10000u);
    uint32_t active = (cr & 0x30u) | 4u | (sector << 8);
    io.bus.write(io.bus.ctx, CR, active);
    io.bus.write(io.bus.ctx, CR, active | 0x80u);
    io.bus.sync(io.bus.ctx);
    f->state = 1;
    return FH_PENDING;
}

enum fh_result fu_poll(struct fu_install *f, struct fb_image *out)
{
    if (!f || !out || f->state < 1 || f->state > 3)
    {
        return FH_REJECT;
    }
    struct fh_io x = f->io.bus;
    if (f->state == 1)
    {
        uint32_t sr = x.read(x.ctx, SR);
        if (sr & 7u)
        {
            return FH_PENDING;
        }
        if (!(sr & (ERR | 0x10000u)))
        {
            return FH_PENDING;
        }
        x.sync(x.ctx);
        x.write(x.ctx, CR, f->control & ~1u);
        if (f->control & 1u)
        {
            x.write(x.ctx, CR, f->control);
        }
        x.sync(x.ctx);
        if (sr & ERR)
        {
            f->state = 6;
            return FH_ERROR;
        }
        f->io.fresh(x.ctx, f->sector_base, 0x20000);
        f->state = 2;
    }
    if (f->state == 2)
    {
        for (unsigned n = 0; n < 32 && f->checked < 0x20000u;
             ++n, f->checked += 4)
        {
            if (x.read(x.ctx, f->sector_base + f->checked) != UINT32_MAX)
            {
                f->state = 6;
                return FH_ERROR;
            }
        }
        if (x.read(x.ctx, SR) & ERR)
        {
            f->state = 6;
            return FH_ERROR;
        }
        if (f->checked < 0x20000u)
        {
            return FH_PENDING;
        }
        f->state = 3;
        return word(f);
    }
    enum fh_result z = fh_poll(&f->program);
    if (z == FH_PENDING)
    {
        return z;
    }
    if (z != FH_DONE)
    {
        f->state = 6;
        return z;
    }
    if (++f->word < 67)
    {
        return word(f);
    }
    f->io.fresh(x.ctx, f->layout.base - 32, 2144);
    struct fb_image actual;
    if (FU_MUTANT == 2)
    {
        actual = f->expected;
    }
    else if (!bs_read_image(f->read, f->read_ctx, &f->policy, f->layout,
                            f->expected.core, &actual))
    {
        f->state = 6;
        return FH_ERROR;
    }
    for (unsigned i = 0; i < 32; ++i)
    {
        if (actual.digest[i] != f->expected.digest[i])
        {
            f->state = 6;
            return FH_ERROR;
        }
    }
    *out = actual;
    f->state = 4;
    return FH_DONE;
}
