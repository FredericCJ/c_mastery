#ifndef MF_ORDER_H
#define MF_ORDER_H
#include <stdint.h>
/* Comparative transfer queue. One fixed epoch; all admitted deadlines lie in
 * its forward half-range. Does not replace generation-owned MfTimers. */
typedef struct { uint32_t at,identity; } MfOrderEntry;
typedef struct { MfOrderEntry entries[16];unsigned count,heap,reverse_ties;
                 uint32_t epoch;uint64_t comparisons,moves; } MfOrder;
void mf_order_init(MfOrder *,uint32_t,unsigned heap,unsigned reverse_ties);
int mf_order_push(MfOrder *,MfOrderEntry);
int mf_order_pop(MfOrder *,MfOrderEntry *);
#endif
