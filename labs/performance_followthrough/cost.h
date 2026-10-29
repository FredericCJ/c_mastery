#ifndef PERFORMANCE_FOLLOWTHROUGH_COST_H
#define PERFORMANCE_FOLLOWTHROUGH_COST_H
#include <stddef.h>
#include <stdint.h>
struct pf_bytes { const unsigned char *data; size_t length; unsigned identity; };
struct pf_work { uint64_t calls,bytes,shifts,placements; unsigned available; };
/* n<=256, lengths<=1024. Seeded counters accumulate; saturation invalidates
 * the ledger, never the correctly sorted result. All admission precedes writes. */
int pf_sort(struct pf_bytes *,size_t,unsigned binary,struct pf_work *);
struct pf_slot { unsigned valid,dirty,block,age; };
struct pf_cache { struct pf_slot slots[2]; unsigned tick,fifo; uint64_t reads,writes,useful; };
int pf_cache(struct pf_cache *,unsigned item,unsigned writing);
int pf_flush(struct pf_cache *);
/* Seconds nonnegative; nanoseconds<1e9. Publish only representable intervals. */
int pf_interval(uint64_t,unsigned,uint64_t,unsigned,uint64_t *);
struct pf_event { uint64_t raw,enabled,running,id,group; };
struct pf_ratio { uint64_t numerator,denominator; };
int pf_scale(struct pf_event,struct pf_ratio *);
int pf_cpi(struct pf_event instructions,struct pf_event cycles,struct pf_ratio *);
struct pf_hook { unsigned task,function,enter; uint64_t time; };
struct pf_profile { uint64_t inclusive[8],exclusive[8],calls[8]; unsigned complete; };
/* At most4 contexts,8 functions,16 nested invocations/context,128 events.
 * Any loss/unmatched exit/open frame invalidates exact attribution. */
int pf_profile(const struct pf_hook *,size_t,unsigned dropped,struct pf_profile *);
struct pf_cycle { unsigned trcena,nocyc,owned,enabled,halted; uint32_t start,end; uint64_t bound,hz,overhead; };
enum { PF_CYCLE_OK,PF_CYCLE_UNKNOWN,PF_CYCLE_ABSENT,PF_CYCLE_UNOWNED,PF_CYCLE_DISABLED,PF_CYCLE_AMBIGUOUS,PF_CYCLE_BOUND,PF_CYCLE_HALTED,PF_CYCLE_OVERFLOW };
struct pf_cycle_result { uint32_t delta; struct pf_ratio ns,sensitivity_ns; unsigned sensitivity_valid; };
int pf_cycle(struct pf_cycle,struct pf_cycle_result *);
#endif
