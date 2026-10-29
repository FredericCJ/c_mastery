#include <stddef.h>

/* Explicit audited compiler helpers, no hosted runtime and no allocation. */
void __aeabi_memcpy(void *to, const void *from, size_t n)
{
    volatile unsigned char *d = to;
    const volatile unsigned char *s = from;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
}

void __aeabi_memcpy4(void *to, const void *from, size_t n)
{
    __aeabi_memcpy(to, from, n);
}

void __aeabi_memclr(void *to, size_t n)
{
    volatile unsigned char *d = to;
    for (size_t i = 0; i < n; i++)
        d[i] = 0;
}

void __aeabi_memclr4(void *to, size_t n)
{
    __aeabi_memclr(to, n);
}

void __aeabi_memset(void *to, size_t n, int c)
{
    volatile unsigned char *d = to;
    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)c;
}

void __aeabi_memset4(void *to, size_t n, int c)
{
    __aeabi_memset(to, n, c);
}

void model_entry(void)
{
    for (;;)
    {
        __asm__ volatile("");
    }
}
