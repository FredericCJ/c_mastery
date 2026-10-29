#ifndef CMASTERY_CONSTRAINED_TIMING_H
#define CMASTERY_CONSTRAINED_TIMING_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "CT_OCTET_CODEC_PROFILE");

enum
{
    CT_OK,
    CT_INVALID,
    CT_RANGE,
    CT_AMBIGUOUS,
    CT_BUSY,
    CT_DIFFERENT
};

enum
{
    CT_EQUAL,
    CT_DUE,
    CT_FUTURE
};

enum
{
    CT_CPU,
    CT_ISR,
    CT_PREEMPTED,
    CT_DMA,
    CT_BLOCKED
};

/* bits is exactly8 or16; outputs live/disjoint; refusal preserves output.
   Classification assumes the true separation is strictly below half range.
   Residue elapsed instead requires less than one full wrap, supplied externally. */
int ct_timeout(uint64_t, unsigned, uint64_t, uint32_t *);
int ct_order(unsigned, uint32_t, uint32_t, unsigned *);
int ct_elapsed(unsigned, uint32_t, uint32_t, int, uint32_t *);

struct ct_stamp
{
    uint64_t boot, epoch, quantum_ns;
    uint32_t low;
    unsigned bits;
};

struct ct_clock
{
    _Atomic uint64_t sequence, boot, epoch, quantum_ns;
    _Atomic uint32_t low;
    _Atomic unsigned bits;
};

typedef void (*ct_hook_fn)(void *, unsigned);
/* Fresh/quiescent init. All live shared fields are accessed atomically; SC
   sequence and data prevent a torn successful snapshot. No sequence wrap.
   Hook may publish at phase0(after epoch) or1(after fields); it may not destroy
   the clock/output/context and must terminate. No lock-free bound is claimed. */
int ct_clock_init(struct ct_clock *, struct ct_stamp);
int ct_snapshot(struct ct_clock *, unsigned, ct_hook_fn, void *,
                struct ct_stamp *);
int ct_publish(struct ct_clock *, struct ct_stamp);
int ct_duration(struct ct_stamp, struct ct_stamp, uint64_t *);
int ct_encode(struct ct_stamp, unsigned char[40]);
int ct_decode(const unsigned char *, size_t, struct ct_stamp *);

struct ct_slice
{
    uint64_t begin, end;
    unsigned kind;
};

struct ct_metrics
{
    uint64_t latency, elapsed, response, cpu, isr, preempted, dma, blocked;
};

/* Slices exactly partition[start,finish); CPU excludes all four other kinds. */
int ct_metrics(uint64_t, uint64_t, uint64_t, const struct ct_slice *, size_t,
               struct ct_metrics *);

struct ct_offset
{
    unsigned negative;
    uint64_t magnitude;
};

int ct_jitter(uint64_t, uint64_t, struct ct_offset *);

struct ct_stats
{
    uint64_t minimum, maximum, sum, mean_integer, mean_remainder;
    size_t count, longest_exceedance_run, exceedance_runs;
};

int ct_stats(const uint64_t *, size_t, uint64_t, struct ct_stats *);
#endif
