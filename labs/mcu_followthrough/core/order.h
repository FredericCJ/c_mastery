#ifndef MF_ORDER_H
#define MF_ORDER_H
#include <stdint.h>
/* Comparative transfer queue. One fixed epoch; all admitted deadlines lie in
 * its forward half-range. Does not replace generation-owned MfTimers. */
struct mf_order_entry { uint32_t at,identity; };
struct mf_order { struct mf_order_entry entries[16];unsigned count,heap,reverse_ties;
                 uint32_t epoch;uint64_t comparisons,moves; };
void mf_order_init(struct mf_order *,uint32_t,unsigned heap,unsigned reverse_ties);
int mf_order_push(struct mf_order *,struct mf_order_entry);
int mf_order_pop(struct mf_order *,struct mf_order_entry *);
#endif
