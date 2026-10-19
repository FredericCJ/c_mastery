#include <stddef.h>
/* Explicit freestanding byte operations required by the imported algo.c.
 * Volatile accesses keep compiler loop recognition from generating recursion.
 * memcpy requires disjoint valid ranges; memset requires writable n bytes. */
void *memset(void *target, int value, size_t n) {
    volatile unsigned char *p = target;
    for (size_t i = 0; i < n; ++i) p[i] = (unsigned char)value;
    return target;
}
void *memcpy(void *target, const void *source, size_t n) {
    volatile unsigned char *to = target;
    const volatile unsigned char *from = source;
    for (size_t i = 0; i < n; ++i) to[i] = from[i];
    return target;
}
