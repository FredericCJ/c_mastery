/*
 * Reference header for the header-definitions lab.
 *
 * hh_mix and hh_state have internal linkage: every including translation unit
 * gets its own copy of the helper and of the function-local static counter.
 * hh_read is the one external definition (headers.c).
 *
 * Mutation hooks for the campaign, each a deliberate defect:
 *   HH_MUTANT_HELPER  hh_mix drops the "+ 1"
 *   HH_MUTANT_SHARED  hh_state returns one external counter shared by all units
 */
#ifndef HH_HEADERS_H
#define HH_HEADERS_H
#include <stddef.h>

typedef unsigned (*hh_fn)(unsigned);

/* 3 * value + 1 in unsigned arithmetic; wraps modulo UINT_MAX + 1. */
static inline unsigned hh_mix(unsigned value)
{
#ifdef HH_MUTANT_HELPER
    return value * 3u;
#else
    return value * 3u + 1u;
#endif
}

#ifdef HH_MUTANT_SHARED
extern unsigned hh_shared_state;

static inline unsigned *hh_state(void)
{
    return &hh_shared_state;
}
#else
/* One zero-initialized counter per translation unit, living for the program. */
static inline unsigned *hh_state(void)
{
    static unsigned value;
    return &value;
}
#endif

/*
 * Copies values[index] to *out and returns 0 when values and out are nonnull
 * and index < length; otherwise returns -1 and leaves *out unchanged.
 */
int hh_read(const int *values, size_t length, size_t index, int *out);
#endif
