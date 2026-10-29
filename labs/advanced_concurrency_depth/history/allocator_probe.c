#include <stddef.h>
extern void *malloc(size_t);
extern void free(void *);

void *ah_allocate_node(size_t n)
{
    return malloc(n);
}

void ah_release_node(void *p)
{
    free(p);
}
