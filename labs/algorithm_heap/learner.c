#include "heap.h"

int ah_trace(uint32_t *a, uint32_t n, uint32_t capacity, uint32_t p,
             unsigned arity, unsigned minimum, struct ah_stats *s,
             struct ah_trace *t)
{
    (void)a;
    (void)n;
    (void)capacity;
    (void)p;
    (void)arity;
    (void)minimum;
    (void)s;
    (void)t;
    return 7;
}

void ah_heap_init(struct cd_heap *h, unsigned m)
{
    (void)h;
    (void)m;
}

enum cd_status ah_heap_insert(struct cd_heap *h, int p, unsigned i,
                              struct cd_heap_handle *out)
{
    (void)h;
    (void)p;
    (void)i;
    (void)out;
    return CD_FAILED;
}

enum cd_status ah_heap_take(struct cd_heap *h, int *p, unsigned *i)
{
    (void)h;
    (void)p;
    (void)i;
    return CD_FAILED;
}

enum cd_status ah_heap_get(const struct cd_heap *h, struct cd_heap_handle k,
                           int *p, unsigned *i)
{
    (void)h;
    (void)k;
    (void)p;
    (void)i;
    return CD_FAILED;
}

enum cd_status ah_heap_change(struct cd_heap *h, struct cd_heap_handle k, int p)
{
    (void)h;
    (void)k;
    (void)p;
    return CD_FAILED;
}

enum cd_status ah_heap_build(struct cd_heap *h, const int *a, size_t n,
                             unsigned m)
{
    (void)h;
    (void)a;
    (void)n;
    (void)m;
    return CD_FAILED;
}

/* Prior artifact is separately required; these unsolved import stubs are intentional. */
void cd_heap_init(struct cd_heap *h, unsigned m)
{
    ah_heap_init(h, m);
}

enum cd_status cd_heap_insert(struct cd_heap *h, int p, unsigned i,
                              struct cd_heap_handle *out)
{
    return ah_heap_insert(h, p, i, out);
}

enum cd_status cd_heap_take(struct cd_heap *h, int *p, unsigned *i)
{
    return ah_heap_take(h, p, i);
}

enum cd_status cd_heap_change(struct cd_heap *h, struct cd_heap_handle k, int p)
{
    return ah_heap_change(h, k, p);
}

enum cd_status cd_heap_build(struct cd_heap *h, const int *a, size_t n,
                             unsigned m)
{
    return ah_heap_build(h, a, n, m);
}
