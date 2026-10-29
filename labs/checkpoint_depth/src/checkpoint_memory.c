/* Intentionally unsolved: preserve learner work separately from assessor code. */
#include "checkpoint_memory.h"

enum cpm_status cpm_arena_init(struct cpm_arena *a, unsigned char *b, size_t c,
                               size_t p, size_t r)
{
    (void)a;
    (void)b;
    (void)c;
    (void)p;
    (void)r;
    return CPM_INVALID;
}

enum cpm_status cpm_reserve(struct cpm_arena *a, size_t n, struct cpm_span *s)
{
    (void)a;
    (void)n;
    (void)s;
    return CPM_INVALID;
}

enum cpm_status cpm_view(const struct cpm_arena *a, struct cpm_span s,
                         const unsigned char **o)
{
    (void)a;
    (void)s;
    (void)o;
    return CPM_INVALID;
}

enum cpm_status cpm_pin(struct cpm_arena *a, struct cpm_span s)
{
    (void)a;
    (void)s;
    return CPM_INVALID;
}

enum cpm_status cpm_unpin(struct cpm_arena *a)
{
    (void)a;
    return CPM_INVALID;
}

enum cpm_status cpm_reset(struct cpm_arena *a)
{
    (void)a;
    return CPM_INVALID;
}

void cpm_pool_init(struct cpm_pool *p)
{
    (void)p;
}

void cpm_ring_init(struct cpm_ring *q)
{
    (void)q;
}

void cpm_fail_at(struct cpm_pool *p, unsigned o)
{
    (void)p;
    (void)o;
}

enum cpm_status cpm_allocate(struct cpm_pool *p, int v, struct cpm_handle *h)
{
    (void)p;
    (void)v;
    (void)h;
    return CPM_INVALID;
}

enum cpm_status cpm_lookup(const struct cpm_pool *p, struct cpm_handle h,
                           const struct cpm_record **o)
{
    (void)p;
    (void)h;
    (void)o;
    return CPM_INVALID;
}

enum cpm_status cpm_pool_pin(struct cpm_pool *p, struct cpm_handle h)
{
    (void)p;
    (void)h;
    return CPM_INVALID;
}

enum cpm_status cpm_pool_unpin(struct cpm_pool *p, struct cpm_handle h)
{
    (void)p;
    (void)h;
    return CPM_INVALID;
}

enum cpm_status cpm_release(struct cpm_pool *p, struct cpm_handle h)
{
    (void)p;
    (void)h;
    return CPM_INVALID;
}

enum cpm_status cpm_prefix(struct cpm_pool *p, struct cpm_ring *q, const int *v,
                           size_t n, size_t *a)
{
    (void)p;
    (void)q;
    (void)v;
    (void)n;
    (void)a;
    return CPM_INVALID;
}

enum cpm_status cpm_atomic(struct cpm_pool *p, struct cpm_ring *q, const int *v,
                           size_t n)
{
    (void)p;
    (void)q;
    (void)v;
    (void)n;
    return CPM_INVALID;
}

enum cpm_status cpm_pop(struct cpm_ring *q, struct cpm_handle *h)
{
    (void)q;
    (void)h;
    return CPM_INVALID;
}
