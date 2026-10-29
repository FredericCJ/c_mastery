#include "region.h"

void bf_cohort_init(struct bf_cohort *c)
{
    (void)c;
}

int bf_cohort_batch(struct bf_cohort *c, const unsigned char *w, size_t n,
                    unsigned a, unsigned b, unsigned *out)
{
    (void)c;
    (void)w;
    (void)n;
    (void)a;
    (void)b;
    (void)out;
    return MB_INVALID;
}

int bf_cohort_reset(struct bf_cohort *c)
{
    (void)c;
    return MB_INVALID;
}

void bf_tree_init(struct bf_region_tree *t)
{
    (void)t;
}

int bf_tree_child_reset(struct bf_region_tree *t, unsigned i)
{
    (void)t;
    (void)i;
    return MB_INVALID;
}

int bf_tree_reset(struct bf_region_tree *t)
{
    (void)t;
    return MB_INVALID;
}

int bf_slice_init(struct bf_slice *s, unsigned char *b, size_t n, size_t o,
                  size_t c)
{
    (void)s;
    (void)b;
    (void)n;
    (void)o;
    (void)c;
    return MB_INVALID;
}

int bf_slice_alloc(struct bf_slice *s, size_t n, size_t a,
                   struct bf_slice_span *out)
{
    (void)s;
    (void)n;
    (void)a;
    (void)out;
    return MB_INVALID;
}

int bf_slice_read(const struct bf_slice *s, struct bf_slice_span h, size_t i,
                  unsigned char *out)
{
    (void)s;
    (void)h;
    (void)i;
    (void)out;
    return MB_INVALID;
}

int bf_slice_reset(struct bf_slice *s)
{
    (void)s;
    return MB_INVALID;
}

int bf_growth_init(struct bf_growth *g, struct bf_chunk *c,
                   bf_acquire_chunk_fn a, bf_release_chunk_fn r, void *x)
{
    (void)g;
    (void)c;
    (void)a;
    (void)r;
    (void)x;
    return MB_INVALID;
}

int bf_growth_alloc(struct bf_growth *g, size_t n, size_t a,
                    struct bf_growth_span *out)
{
    (void)g;
    (void)n;
    (void)a;
    (void)out;
    return MB_INVALID;
}

int bf_growth_read(const struct bf_growth *g, struct bf_growth_span s, size_t i,
                   unsigned char *out)
{
    (void)g;
    (void)s;
    (void)i;
    (void)out;
    return MB_INVALID;
}

int bf_growth_reset(struct bf_growth *g)
{
    (void)g;
    return MB_INVALID;
}

void bf_retry_init(struct bf_retry_region *r)
{
    (void)r;
}

int bf_retry_reserve(struct bf_retry_region *r, size_t n, size_t a,
                     struct bf_borrow *out)
{
    (void)r;
    (void)n;
    (void)a;
    (void)out;
    return B2_INVALID;
}

int bf_retry_register(struct bf_retry_region *r, bf_close_fn f, void *c)
{
    (void)r;
    (void)f;
    (void)c;
    return B2_INVALID;
}

int bf_retry_reset(struct bf_retry_region *r)
{
    (void)r;
    return B2_INVALID;
}
