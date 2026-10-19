#ifndef HH_HEADERS_H
#define HH_HEADERS_H
#include <stddef.h>
typedef unsigned (*HhFunction)(unsigned);
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
static inline unsigned *hh_state(void) { return &hh_shared_state; }
#else
static inline unsigned *hh_state(void) { static unsigned value; return &value; }
#endif
int hh_read(const int *values, size_t length, size_t index, int *out);
#endif
