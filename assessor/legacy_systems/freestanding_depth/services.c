#include "services.h"
#include <limits.h>

enum fr_status fr_service_init(struct fr_service *s, fr_output f, void *p)
{
    if (!s || !f)
    {
        return FR_ARGUMENT;
    }
    s->used = 0;
    s->offset = 0;
    s->ready = 0;
    s->active = 0;
    s->output = f;
    s->context = p;
    return FR_OK;
}

enum fr_status fr_service_ready(struct fr_service *s)
{
    if (!s || !s->output)
    {
        return FR_ARGUMENT;
    }
    s->ready = 1;
    return FR_OK;
}

enum fr_status fr_service_begin(struct fr_service *s, const unsigned char *b,
                                size_t n)
{
    if (!s || (!b && n) || !s->output)
    {
        return FR_ARGUMENT;
    }
    if (!s->ready)
    {
        return FR_NOT_READY;
    }
    if (s->active)
    {
        return FR_BLOCKED;
    }
    if (n > sizeof s->bytes)
    {
        return FR_RANGE;
    }
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[i] = b[i];
    }
    s->used = n;
    s->offset = 0;
    s->active = n != 0;
    return FR_OK;
}

enum fr_status fr_service_pump(struct fr_service *s)
{
    if (!s || !s->output || s->offset > s->used || s->used > sizeof s->bytes)
    {
        return FR_ARGUMENT;
    }
    if (!s->ready)
    {
        return FR_NOT_READY;
    }
    if (!s->active)
    {
        return FR_OK;
    }
    size_t left = s->used - s->offset;
    struct fr_progress p = s->output(s->context, s->bytes + s->offset, left);
    if (p.accepted > left ||
        (p.status != FR_OK && p.status != FR_BLOCKED && p.status != FR_IO) ||
        (!p.accepted && p.status == FR_OK))
    {
        s->active = 0;
        return FR_IO;
    }
#ifndef FR_MUTANT_RESEND
    s->offset += p.accepted;
#endif
    if (p.status == FR_IO)
    {
        s->active = 0;
        return FR_IO;
    }
    if (s->offset == s->used)
    {
        s->active = 0;
        return FR_OK;
    }
    return FR_BLOCKED;
}

enum fr_status fr_heap_extend(struct fr_service *s, size_t n, void **p)
{
    (void)s;
    (void)n;
    (void)p;
#ifdef FR_MUTANT_REFUSAL
    return FR_OK;
#else
    return FR_UNSUPPORTED;
#endif
}

enum fr_status fr_allocate(struct fr_arena *a, size_t n, size_t align,
                           void **out)
{
    if (!a || !out || !n || !align || (align & (align - 1)) ||
        align > _Alignof(max_align_t))
    {
        return FR_ARGUMENT;
    }
    if (a->used > sizeof a->bytes)
    {
        return FR_RANGE;
    }
    size_t pad = (align - (a->used & (align - 1))) & (align - 1);
    if (pad > sizeof a->bytes - a->used || n > sizeof a->bytes - a->used - pad)
    {
        return FR_EXHAUSTED;
    }
    size_t start = a->used + pad;
    *out = a->bytes + start;
    a->used = start + n;
    return FR_OK;
}

static int valid(struct fr_region r, size_t cap)
{
    return r.begin <= r.end && r.end <= cap;
}

static int intersects(struct fr_region a, struct fr_region b)
{
    return a.begin < a.end && b.begin < b.end && a.begin < b.end &&
           b.begin < a.end;
}

enum fr_status fr_startup(unsigned char *ram, size_t n,
                          const unsigned char *rom, size_t m,
                          const struct fr_start_plan *p, size_t cut)
{
    if (!ram || !rom || !p)
    {
        return FR_ARGUMENT;
    }
    struct fr_region r[4] = {p->data, p->bss, p->retained, p->stack};
    for (unsigned i = 0; i < 4; ++i)
    {
        if (!valid(r[i], n))
        {
            return FR_RANGE;
        }
        for (unsigned j = 0; j < i; ++j)
        {
            if (intersects(r[i], r[j]))
            {
                return FR_OVERLAP;
            }
        }
    }
    if (p->stack.begin == p->stack.end || (p->stack.begin & 7) ||
        (p->stack.end & 7))
    {
        return FR_ARGUMENT;
    }
    size_t count = p->data.end - p->data.begin;
    if (p->load > m || count > m - p->load)
    {
        return FR_RANGE;
    }
    size_t done = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (done == cut)
        {
            return FR_NOT_READY;
        }
        ram[p->data.begin + i] = rom[p->load + i];
        ++done;
    }
    for (size_t i = p->bss.begin; i < p->bss.end; ++i)
    {
        if (done == cut)
        {
            return FR_NOT_READY;
        }
        ram[i] = 0;
        ++done;
    }
#ifdef FR_MUTANT_RETAIN
    for (size_t i = p->retained.begin; i < p->retained.end; ++i)
    {
        ram[i] = 0;
    }
#endif
    return FR_OK;
}

static int live(const struct fr_tasks *t, struct fr_handle h)
{
    return t && h.index < 4 && t->slot[h.index].active &&
           t->slot[h.index].generation == h.generation;
}

enum fr_status fr_task_create(struct fr_tasks *t, unsigned value,
                              struct fr_handle *out)
{
    if (!t || !out)
    {
        return FR_ARGUMENT;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if (!t->slot[i].active && t->slot[i].generation != UINT_MAX)
        {
            struct fr_task *p = &t->slot[i];
            ++p->generation;
            p->value = value;
            p->hooks = 0;
            p->active = 1;
            out->index = i;
            out->generation = p->generation;
            return FR_OK;
        }
    }
    return FR_EXHAUSTED;
}

enum fr_status fr_task_hook(struct fr_tasks *t, struct fr_handle h,
                            const struct fr_service *s)
{
    if (!live(t, h) || !s)
    {
        return FR_ARGUMENT;
    }
    if (!s->ready)
    {
        return FR_NOT_READY;
    }
    if (t->slot[h.index].hooks)
    {
        return FR_ARGUMENT;
    }
    ++t->slot[h.index].hooks;
    return FR_OK;
}

enum fr_status fr_task_read(const struct fr_tasks *t, struct fr_handle h,
                            unsigned *out)
{
    if (!live(t, h) || !out)
    {
        return FR_ARGUMENT;
    }
#ifdef FR_MUTANT_TLS
    *out = t->slot[0].value;
#else
    *out = t->slot[h.index].value;
#endif
    return FR_OK;
}

enum fr_status fr_task_destroy(struct fr_tasks *t, struct fr_handle h)
{
    if (!live(t, h))
    {
        return FR_ARGUMENT;
    }
    t->slot[h.index].value = 0;
    t->slot[h.index].hooks = 0;
    t->slot[h.index].active = 0;
    return FR_OK;
}
