/* Explicit selected Arm EABI aggregate-zero helper observed in the Clang
 * target image. This is not a container or allocator implementation. */
#include <stddef.h>

void __aeabi_memclr4(void *destination, size_t n)
{
    unsigned char *p = destination;
    for (size_t i = 0; i < n; ++i)
        p[i] = 0;
}

void __aeabi_memcpy4(void *destination, const void *source, size_t n)
{
    unsigned char *d = destination;
    const unsigned char *s = source;
    for (size_t i = 0; i < n; ++i)
        d[i] = s[i];
}
