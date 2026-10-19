#ifndef PERFORMANCE_FOLLOWTHROUGH_COST_H
#define PERFORMANCE_FOLLOWTHROUGH_COST_H
#include <stddef.h>
#include <stdint.h>
typedef struct { const unsigned char *data; size_t length; unsigned identity; } PFBytes;
typedef struct { uint64_t calls,bytes,shifts,placements; unsigned available; } PFWork;
/* n<=256, lengths<=1024. Seeded counters accumulate; saturation invalidates
 * the ledger, never the correctly sorted result. All admission precedes writes. */
int pf_sort(PFBytes *,size_t,unsigned binary,PFWork *);
typedef struct { unsigned valid,dirty,block,age; } PFSlot;
typedef struct { PFSlot slots[2]; unsigned tick,fifo; uint64_t reads,writes,useful; } PFCache;
int pf_cache(PFCache *,unsigned item,unsigned writing);
int pf_flush(PFCache *);
/* Seconds nonnegative; nanoseconds<1e9. Publish only representable intervals. */
int pf_interval(uint64_t,unsigned,uint64_t,unsigned,uint64_t *);
typedef struct { uint64_t raw,enabled,running,id,group; } PFEvent;
typedef struct { uint64_t numerator,denominator; } PFRatio;
int pf_scale(PFEvent,PFRatio *);
int pf_cpi(PFEvent instructions,PFEvent cycles,PFRatio *);
typedef struct { unsigned task,function,enter; uint64_t time; } PFHook;
typedef struct { uint64_t inclusive[8],exclusive[8],calls[8]; unsigned complete; } PFProfile;
/* At most4 contexts,8 functions,16 nested invocations/context,128 events.
 * Any loss/unmatched exit/open frame invalidates exact attribution. */
int pf_profile(const PFHook *,size_t,unsigned dropped,PFProfile *);
typedef struct { unsigned trcena,nocyc,owned,enabled,halted; uint32_t start,end; uint64_t bound,hz,overhead; } PFCycle;
enum { PF_CYCLE_OK,PF_CYCLE_UNKNOWN,PF_CYCLE_ABSENT,PF_CYCLE_UNOWNED,PF_CYCLE_DISABLED,PF_CYCLE_AMBIGUOUS,PF_CYCLE_BOUND,PF_CYCLE_HALTED,PF_CYCLE_OVERFLOW };
typedef struct { uint32_t delta; PFRatio ns,sensitivity_ns; unsigned sensitivity_valid; } PFCycleResult;
int pf_cycle(PFCycle,PFCycleResult *);
#endif
