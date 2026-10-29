#include "machine.h"

void *mi_move(void *d, const void *s, size_t n)
{
    (void)s;
    (void)n;
    return d;
}

int mi_compare(const void *a, const void *b, size_t n)
{
    (void)a;
    (void)b;
    (void)n;
    return 0;
}
