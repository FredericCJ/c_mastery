#include "memory.h"

/* Learner implementation: this file intentionally does not complete the task. */
enum cpx_status cpx_init(struct cpx_store *s, struct cpx_backend b, unsigned c,
                         unsigned n)
{
    (void)s;
    (void)b;
    (void)c;
    (void)n;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_close(struct cpx_store *s)
{
    (void)s;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_acknowledge(struct cpx_store *s, uint64_t c, int q)
{
    (void)s;
    (void)c;
    (void)q;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_allocate(struct cpx_store *s, const struct cpx_record *r,
                             struct cpx_handle *h)
{
    (void)s;
    (void)r;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_lookup(struct cpx_store *s, struct cpx_handle h,
                           const struct cpx_record **r)
{
    (void)s;
    (void)h;
    (void)r;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_pin(struct cpx_store *s, struct cpx_handle h)
{
    (void)s;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_unpin(struct cpx_store *s, struct cpx_handle h)
{
    (void)s;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_release(struct cpx_store *s, struct cpx_handle h)
{
    (void)s;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_relocate(struct cpx_store *s)
{
    (void)s;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_grow(struct cpx_store *s, unsigned c)
{
    (void)s;
    (void)c;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_push(struct cpx_store *s, struct cpx_handle h)
{
    (void)s;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_pop(struct cpx_store *s, struct cpx_handle *h)
{
    (void)s;
    (void)h;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_batch(struct cpx_store *s, const struct cpx_record *r,
                          size_t n, size_t *a)
{
    (void)s;
    (void)r;
    (void)n;
    (void)a;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_lease_begin(struct cpx_lease *l, struct cpm_arena *a,
                                struct cpm_span s, uint64_t c)
{
    (void)l;
    (void)a;
    (void)s;
    (void)c;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_lease_cancel(struct cpx_lease *l)
{
    (void)l;
    return CPX_NOT_IMPLEMENTED;
}

enum cpx_status cpx_lease_complete(struct cpx_lease *l, uint64_t c, int q)
{
    (void)l;
    (void)c;
    (void)q;
    return CPX_NOT_IMPLEMENTED;
}
