#include "performance.h"

/* Incomplete learner core; no assessor source is selected implicitly. */
void bf_add(const uint32_t *a, const uint32_t *b, uint32_t *o, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)n;
}

void bf_add_disjoint(const uint32_t *restrict a, const uint32_t *restrict b,
                     uint32_t *restrict o, size_t n)
{
    (void)a;
    (void)b;
    (void)o;
    (void)n;
}

void bf_add_inplace(uint32_t *a, const uint32_t *b, size_t n)
{
    (void)a;
    (void)b;
    (void)n;
}

void bf_add_unroll(const uint32_t *a, const uint32_t *b, uint32_t *o, size_t n,
                   unsigned l)
{
    (void)a;
    (void)b;
    (void)o;
    (void)n;
    (void)l;
}

int bf_select(int f, int s, int force)
{
    (void)f;
    (void)s;
    (void)force;
    return 0;
}

void bf_dispatch(const uint32_t *a, const uint32_t *b, uint32_t *o, size_t n,
                 int f, int s, int force)
{
    (void)f;
    (void)s;
    (void)force;
    bf_add(a, b, o, n);
}

int bf_clamp_branch(int x)
{
    (void)x;
    return 0;
}

int bf_clamp_select(int x)
{
    (void)x;
    return 0;
}

uint32_t bf_recurrence(const uint32_t *a, size_t n)
{
    (void)a;
    (void)n;
    return 0;
}

float bf_reduce(const float *a, size_t n)
{
    (void)a;
    (void)n;
    return 0;
}

void bf_to_soa(const struct bf_record *a, size_t n, uint32_t *id, uint32_t *v)
{
    (void)a;
    (void)n;
    (void)id;
    (void)v;
}

int bf_delete_soa(uint32_t *id, uint32_t *v, size_t *n, uint32_t k)
{
    (void)id;
    (void)v;
    (void)n;
    (void)k;
    return 0;
}

void bf_blocked(const uint32_t *in, size_t r, size_t c, size_t s, uint32_t *o,
                size_t os, size_t b)
{
    (void)in;
    (void)r;
    (void)c;
    (void)s;
    (void)o;
    (void)os;
    (void)b;
}
