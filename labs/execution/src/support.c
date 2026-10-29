#include <stddef.h>

/* TODO: original byte loops. Do not silently add -lc/-lgcc/-latomic. */
void *memcpy(void *d, const void *s, size_t n)
{
    (void)s;
    (void)n;
    return d;
}

void *memset(void *d, int v, size_t n)
{
    (void)v;
    (void)n;
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    (void)s;
    (void)n;
    return d;
}

void __aeabi_memclr(void *d, size_t n)
{
    (void)d;
    (void)n;
}

void __aeabi_memclr4(void *d, size_t n)
{
    (void)d;
    (void)n;
}

void __aeabi_memclr8(void *d, size_t n)
{
    (void)d;
    (void)n;
}

void __aeabi_memcpy(void *d, const void *s, size_t n)
{
    (void)d;
    (void)s;
    (void)n;
}

void __aeabi_memcpy4(void *d, const void *s, size_t n)
{
    (void)d;
    (void)s;
    (void)n;
}

void __aeabi_memcpy8(void *d, const void *s, size_t n)
{
    (void)d;
    (void)s;
    (void)n;
}
