#include "fault.h"
#include "frame.h"
#ifndef DF_MUTANT
#define DF_MUTANT 0
#endif
void df_init(struct df_capture *c)
{
    atomic_init(&c->state, 0);
    atomic_init(&c->secondary, 0);
    c->record = (struct df_record){0};
}

int df_capture(struct df_capture *c, const struct df_input *i,
               void (*hook)(void *), void *context)
{
    if (!c || !i || i->readable > 1 || i->aligned > 1 || i->stacking_fault > 1)
    {
        return DF_INVALID;
    }
    unsigned empty = 0;
    if (!atomic_compare_exchange_strong(&c->state, &empty, 1))
    {
        if (DF_MUTANT == 5)
        {
            c->record.cfsr = i->cfsr;
        }
        atomic_store(&c->secondary, 1);
        return DF_AGAIN;
    }
    struct df_record r = {.version = 1,
                          .valid = DF_STATUS,
                          .cfsr = i->cfsr,
                          .hfsr = i->hfsr,
                          .exc_return = i->exc_return,
                          .frame_status = DF_INCOMPLETE};
    if ((i->cfsr & 0x8000u) || DF_MUTANT == 1)
    {
        r.valid |= DF_BUS_ADDRESS;
        r.bfar = i->bfar;
    }
    if (i->cfsr & 0x80u)
    {
        r.valid |= DF_MEMORY_ADDRESS;
        r.mmfar = i->mmfar;
    }
    if (i->image)
    {
        for (unsigned n = 0; n < 32; ++n)
        {
            r.image[n] = i->image[n];
        }
        r.valid |= DF_IMAGE;
    }
    if (hook)
    {
        hook(context);
    }
    if (i->readable)
    {
        struct xf_frame frame;
        int status = xf_frame(
            i->words, i->available, i->stack_address, i->exc_return, 0, 0,
            i->aligned, i->stacking_fault || (i->cfsr & 0x3838u) != 0, &frame);
        r.frame_status = (unsigned)status;
        if (status == XF_OK)
        {
            for (unsigned n = 0; n < 4; ++n)
            {
                r.frame[n] = frame.r[n];
            }
            r.frame[4] = frame.r12;
            r.frame[5] = frame.stacked_lr;
            r.frame[6] = frame.pc;
            r.frame[7] = frame.xpsr;
            r.valid |= DF_FRAME;
        }
    }
    c->record = r;
    atomic_store_explicit(&c->state, 2, memory_order_release);
    return DF_OK;
}

int df_read(const struct df_capture *c, struct df_record *out,
            unsigned *secondary)
{
    if (!c || !out || !secondary)
    {
        return DF_INVALID;
    }
    if (atomic_load_explicit(&c->state, memory_order_acquire) != 2)
    {
        return DF_BUSY;
    }
    *out = c->record;
    *secondary = atomic_load(&c->secondary);
    return DF_OK;
}

int df_flags(const struct df_record *r, struct df_flags *out)
{
    if (!r || !out)
    {
        return DF_INVALID;
    }
    if (r->version != 1)
    {
        return DF_UNSUPPORTED;
    }
    if (!(r->valid & DF_STATUS))
    {
        return DF_INCOMPLETE;
    }
    *out = (struct df_flags){
        (r->cfsr & 0x200u) != 0, (r->valid & DF_BUS_ADDRESS) != 0,
        (r->hfsr & 0x40000000u) != 0, (r->cfsr & 0x400u) != 0};
    return DF_OK;
}

int df_access(const struct df_record *r, const unsigned char image[32],
              uint32_t base, const unsigned char *code, size_t n,
              struct df_access *out)
{
    if (!r || !image || !code || !out)
    {
        return DF_INVALID;
    }
    if (r->version != 1)
    {
        return DF_UNSUPPORTED;
    }
    if ((r->valid & (DF_FRAME | DF_IMAGE | DF_STATUS)) !=
        (DF_FRAME | DF_IMAGE | DF_STATUS))
    {
        return DF_INCOMPLETE;
    }
    for (unsigned j = 0; j < 32; ++j)
    {
        if (DF_MUTANT != 2 && image[j] != r->image[j])
        {
            return DF_IDENTITY;
        }
    }
    uint32_t pc = r->frame[6];
    if (n > UINT32_MAX - base || pc < base || pc - base >= n ||
        n - (pc - base) < 2)
    {
        return DF_RANGE;
    }
    size_t offset = pc - base;
    unsigned instruction = code[offset] | (unsigned)code[offset + 1] << 8;
    unsigned opcode = instruction & 0xf800u;
    if (opcode != 0x6000u && opcode != 0x6800u)
    {
        return DF_UNSUPPORTED;
    }
    unsigned rn = (instruction >> 3) & 7u, rt = instruction & 7u,
             displacement = ((instruction >> 6) & 31u) * 4u;
    if (rn > 3 || (opcode == 0x6000u && rt > 3))
    {
        return DF_INCOMPLETE;
    }
    if (r->frame[rn] > UINT32_MAX - displacement)
    {
        return DF_RANGE;
    }
    struct df_access value = {.pc = pc,
                              .effective_address = r->frame[rn] + displacement,
                              .operand = opcode == 0x6000u ? r->frame[rt] : 0,
                              .write = opcode == 0x6000u};
    uint32_t event = r->cfsr & ~UINT32_C(0x8080);
    value.precise = (event == 2u || event == 0x200u) || DF_MUTANT == 3;
    *out = value;
    return DF_OK;
}

int df_refault(unsigned kind, unsigned ignore, unsigned mpu, unsigned *out)
{
    if (!out || kind > 3 || ignore > 1 || mpu > 1)
    {
        return DF_INVALID;
    }
    *out = kind == 0   ? (ignore ? DF_IGNORED : DF_LOCKUP)
           : kind == 1 ? DF_PENDING
           : kind == 2 ? (mpu ? DF_LOCKUP : DF_IGNORED)
                       : DF_LOCKUP;
    return DF_OK;
}

int df_watermark(const unsigned char *s, size_t n, unsigned char fill,
                 size_t *out)
{
    if (!s || !out)
    {
        return DF_INVALID;
    }
    size_t first = 0;
    while (first < n && s[first] == fill)
    {
        ++first;
    }
    *out = n - first;
    return DF_OK;
}

int df_canary(const unsigned char *p, size_t n, unsigned char value,
              unsigned *out)
{
    if (!p || !out || !n)
    {
        return DF_INVALID;
    }
    unsigned intact = 1;
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i] != value)
        {
            intact = 0;
        }
    }
    *out = intact;
    return DF_OK;
}

int df_budget(const struct df_live_frame *f, size_t n, uint32_t allocated,
              unsigned recursion, unsigned opaque, struct df_budget *out)
{
    if (!f || !out || n > 8 || recursion > 1 || opaque > 1)
    {
        return DF_INVALID;
    }
    if (recursion || opaque)
    {
        return DF_UNKNOWN;
    }
    uint32_t total = 0;
    for (size_t j = 0; j < n; ++j)
    {
        if (f[j].exception > 1 || f[j].extended > 1 || f[j].padding > 1 ||
            (!f[j].exception && (f[j].extended || f[j].padding)))
        {
            return DF_INVALID;
        }
        uint32_t hardware =
            f[j].exception ? (f[j].extended ? 104u : 32u) + 4u * f[j].padding
                           : 0;
        if (DF_MUTANT == 4)
        {
            hardware = f[j].exception ? 32u : 0;
        }
        if (f[j].software > UINT32_MAX - hardware ||
            total > UINT32_MAX - f[j].software - hardware)
        {
            return DF_RANGE;
        }
        total += f[j].software + hardware;
    }
    *out = (struct df_budget){total, total <= allocated};
    return DF_OK;
}
