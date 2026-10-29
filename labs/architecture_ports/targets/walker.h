#ifndef B06_WALKER_H
#define B06_WALKER_H
#include <stdint.h>
enum { PW_OK, PW_INVALID, PW_PAGE_FAULT, PW_ACCESS_FAULT };
struct pw_memory {uint64_t pte[8][512];unsigned present,read_blocked;};
/* Synthetic Sv39+Svade, no optional PTE extensions, serialized immutable tables.
   root is a modeled byte address; table memory is eight pages at 0..32767.
   No actual integer-to-pointer conversion or hardware translation is performed.
   Parameters/tables/output live and disjoint; failure preserves *physical. */
int pw_translate(const struct pw_memory *,uint64_t root,uint64_t virtual_address,
 unsigned supervisor,unsigned sum,unsigned mxr,unsigned operation,uint64_t *physical);
#endif
