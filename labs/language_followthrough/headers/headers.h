/*
 * Self-contained public header: hh_mix and hh_state have internal linkage and
 * are therefore private to each translation unit that includes this file;
 * hh_read is the single external definition, provided by exactly one .c file.
 */
#ifndef HH_HEADERS_H
#define HH_HEADERS_H
#include <stddef.h>

typedef unsigned (*hh_fn)(unsigned);

/*
 * Contract: returns 3 * value + 1 in unsigned (wraparound) arithmetic. No side
 * effects. Scaffold: the body is a placeholder; replace it, keep the signature.
 */
static inline unsigned hh_mix(unsigned value)
{
    (void)value;
    return 0;
}

/*
 * Contract: returns the address of a zero-initialized unsigned that is private
 * to the including translation unit and lives for the whole program. Not a
 * synchronization primitive.
 */
static inline unsigned *hh_state(void)
{
    static unsigned value;
    return &value;
}

/*
 * Copies values[index] to *out and returns 0 when values and out are nonnull
 * and index < length. Otherwise returns -1 and leaves *out unchanged. The
 * caller owns both objects; values must designate at least length live ints.
 * Borrows only until return; constant time; no allocation.
 */
int hh_read(const int *values, size_t length, size_t index, int *out);
#endif
