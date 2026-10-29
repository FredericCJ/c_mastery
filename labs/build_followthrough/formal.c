#include "formal.h"

/* Learner-owned implementation: this file deliberately compiles and fails semantic tests. */
enum bf_status bf_sum(const unsigned *a, size_t e, size_t n, unsigned *o)
{
    (void)a;
    (void)e;
    (void)n;
    (void)o;
    return BF_STUB;
}

enum bf_status bf_sum_pairs(const unsigned *a, size_t e, size_t n, unsigned *o)
{
    (void)a;
    (void)e;
    (void)n;
    (void)o;
    return BF_STUB;
}

enum bf_status bf_signed(const int *a, size_t e, size_t n, int *o)
{
    (void)a;
    (void)e;
    (void)n;
    (void)o;
    return BF_STUB;
}

enum bf_status bf_max(const unsigned *a, size_t e, size_t n, unsigned *o)
{
    (void)a;
    (void)e;
    (void)n;
    (void)o;
    return BF_STUB;
}

enum bf_status bf_write(bf_writer w, void *c, const unsigned char *a, size_t n,
                        unsigned b, size_t *o)
{
    (void)w;
    (void)c;
    (void)a;
    (void)n;
    (void)b;
    (void)o;
    return BF_STUB;
}

unsigned bf_read_clear(unsigned *s)
{
    (void)s;
    return 0;
}
