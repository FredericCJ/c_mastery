#include <stddef.h>
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ || _CALL_ELF != 2
#error This transfer requires actual big-endian PowerPC64 ELFv2.
#endif
/* Reuse the inspected standalone ELFv2 verifier bootstrap, not a target
 * runtime. */
extern long sa_raw_write(const char *, size_t);

static size_t length(const char *s)
{
    size_t n = 0;
    while (s[n])
        ++n;
    return n;
}

int pm_printf(const char *s, ...)
{
    return (int)sa_raw_write(s, length(s));
}

int pm_puts(const char *s)
{
    size_t n = length(s);
    if (sa_raw_write(s, n) != (long)n)
        return -1;
    return (int)sa_raw_write("\n", 1);
}

void *memcpy(void *d, const void *s, size_t n)
{
    volatile unsigned char *out = d;
    const volatile unsigned char *in = s;
    for (size_t i = 0; i < n; ++i)
        out[i] = in[i];
    return d;
}

void *memset(void *d, int v, size_t n)
{
    volatile unsigned char *out = d;
    for (size_t i = 0; i < n; ++i)
        out[i] = (unsigned char)v;
    return d;
}
