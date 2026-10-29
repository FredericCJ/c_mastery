#include "facade.h"

struct bf_record *bf_mutable(struct bf_record *p)
{
    (void)p;
    return 0;
}

const struct bf_record *bf_const(const struct bf_record *p)
{
    (void)p;
    return 0;
}

int bf_binary_int(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}

unsigned bf_binary_unsigned(unsigned a, unsigned b)
{
    (void)a;
    (void)b;
    return 0;
}

int bf_sequence(bf_get_fn a, void *b, bf_get_fn c, void *d, int *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return -1;
}
