#include "metaprog.h"
#ifndef PROBE_CAPACITY
#define PROBE_CAPACITY 4
#endif
_Static_assert(PROBE_CAPACITY>0 && (PROBE_CAPACITY&(PROBE_CAPACITY-1))==0,"capacity must be a positive power of two");
struct probe_storage { uint32_t values[PROBE_CAPACITY]; };
size_t probe(void) { return sizeof(struct probe_storage); }
