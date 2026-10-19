#ifndef CMASTERY_CONSTRAINED_TIMING_H
#define CMASTERY_CONSTRAINED_TIMING_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <limits.h>
_Static_assert(CHAR_BIT==8,"CT_OCTET_CODEC_PROFILE");
enum { CT_OK,CT_INVALID,CT_RANGE,CT_AMBIGUOUS,CT_BUSY,CT_DIFFERENT };
enum { CT_EQUAL,CT_DUE,CT_FUTURE };
enum { CT_CPU,CT_ISR,CT_PREEMPTED,CT_DMA,CT_BLOCKED };
/* bits is exactly8 or16; outputs live/disjoint; refusal preserves output.
   Classification assumes the true separation is strictly below half range.
   Residue elapsed instead requires less than one full wrap, supplied externally. */
int ct_timeout(uint64_t,unsigned,uint64_t,uint32_t *);
int ct_order(unsigned,uint32_t,uint32_t,unsigned *);
int ct_elapsed(unsigned,uint32_t,uint32_t,int,uint32_t *);
typedef struct {uint64_t boot,epoch,quantum_ns;uint32_t low;unsigned bits;} CtStamp;
typedef struct {_Atomic uint64_t sequence,boot,epoch,quantum_ns;_Atomic uint32_t low;_Atomic unsigned bits;} CtClock;
typedef void (*CtHook)(void *,unsigned);
/* Fresh/quiescent init. All live shared fields are accessed atomically; SC
   sequence and data prevent a torn successful snapshot. No sequence wrap.
   Hook may publish at phase0(after epoch) or1(after fields); it may not destroy
   the clock/output/context and must terminate. No lock-free bound is claimed. */
int ct_clock_init(CtClock *,CtStamp);
int ct_snapshot(CtClock *,unsigned,CtHook,void *,CtStamp *);
int ct_publish(CtClock *,CtStamp);
int ct_duration(CtStamp,CtStamp,uint64_t *);
int ct_encode(CtStamp,unsigned char [40]);
int ct_decode(const unsigned char *,size_t,CtStamp *);
typedef struct {uint64_t begin,end;unsigned kind;} CtSlice;
typedef struct {uint64_t latency,elapsed,response,cpu,isr,preempted,dma,blocked;} CtMetrics;
/* Slices exactly partition[start,finish); CPU excludes all four other kinds. */
int ct_metrics(uint64_t,uint64_t,uint64_t,const CtSlice *,size_t,CtMetrics *);
typedef struct {unsigned negative;uint64_t magnitude;} CtOffset;
int ct_jitter(uint64_t,uint64_t,CtOffset *);
typedef struct {uint64_t minimum,maximum,sum,mean_integer,mean_remainder;size_t count,longest_exceedance_run,exceedance_runs;} CtStats;
int ct_stats(const uint64_t *,size_t,uint64_t,CtStats *);
#endif
