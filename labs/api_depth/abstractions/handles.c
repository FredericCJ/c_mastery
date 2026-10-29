#include "handles.h"

/* Intentionally unfinished learner mechanisms. Private implementations are excluded. */
void ah_pool_init(struct ah_pool *p)
{
    (void)p;
}

enum ah_result ah_allocate(struct ah_pool *p, unsigned v, struct ah_handle *h)
{
    (void)p;
    (void)v;
    (void)h;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_read(const struct ah_pool *p, struct ah_handle h, unsigned *v)
{
    (void)p;
    (void)h;
    (void)v;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_release(struct ah_pool *p, struct ah_handle h)
{
    (void)p;
    (void)h;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_enter(struct ah_pool *p)
{
    (void)p;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_leave(struct ah_pool *p)
{
    (void)p;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_reuse(struct ah_pool *p)
{
    (void)p;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_shared_create(unsigned v, struct ah_stats *s,
                                struct ah_shared **o)
{
    (void)v;
    (void)s;
    (void)o;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_retain(struct ah_shared *s)
{
    (void)s;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_drop(struct ah_shared *s)
{
    (void)s;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_weak_retain(struct ah_shared *s)
{
    (void)s;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_weak_drop(struct ah_shared *s)
{
    (void)s;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_promote(struct ah_shared *s)
{
    (void)s;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_counts(struct ah_shared *s, unsigned *a, unsigned *b)
{
    (void)s;
    (void)a;
    (void)b;
    return AH_UNIMPLEMENTED;
}

enum ah_result ah_add(struct ah_shared *s, unsigned v)
{
    (void)s;
    (void)v;
    return AH_UNIMPLEMENTED;
}

unsigned *ah_payload(struct ah_shared *s)
{
    (void)s;
    return 0;
}
