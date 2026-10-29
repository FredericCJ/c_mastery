#ifndef HH_HEADERS_H
#define HH_HEADERS_H
#include <stddef.h>
typedef unsigned (*hh_fn)(unsigned);
/* Replace these bodies; preserve the public signatures and contract. */
static inline unsigned hh_mix(unsigned value) { (void)value; return 0; }
static inline unsigned *hh_state(void) { static unsigned value; return &value; }
int hh_read(const int *values, size_t length, size_t index, int *out);
#endif
