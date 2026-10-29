#include <stddef.h>

/* Explicit AEABI boundary required by the selected Clang aggregate copy.
 * The caller guarantees valid, nonoverlapping ranges. Byte copying does not
 * require stronger alignment than the ABI's advertised four-byte premise.
 * Volatile accesses prevent this original primitive from recursing into itself.
 */
void __aeabi_memcpy4(void *destination, const void *source, size_t n)
{
    volatile unsigned char *d = destination;
    const volatile unsigned char *s = source;
    for (size_t i = 0; i < n; ++i)
        d[i] = s[i];
}
