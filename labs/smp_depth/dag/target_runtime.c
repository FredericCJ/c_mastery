/* Explicit byte helpers for compiler-lowered struct copies/initialization.
 * Valid live regions are required; memcpy regions must not overlap.
 * Volatile stores keep these bootstrap loops from becoming recursive helpers.
 * This file is target support, not the learner's DAG implementation. */
#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t count)
{
    volatile unsigned char *out = destination;
    const unsigned char *in = source;
    for (size_t i = 0; i < count; ++i)
        out[i] = in[i];
    return destination;
}

void *memset(void *destination, int value, size_t count)
{
    volatile unsigned char *out = destination;
    for (size_t i = 0; i < count; ++i)
        out[i] = (unsigned char)value;
    return destination;
}
