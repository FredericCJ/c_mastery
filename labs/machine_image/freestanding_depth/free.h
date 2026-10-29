#ifndef CMASTERY_FREESTANDING_DEPTH_H
#define CMASTERY_FREESTANDING_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum fr_status
{
    FR_OK,
    FR_ARGUMENT,
    FR_RANGE,
    FR_OVERLAP,
    FR_DIVZERO,
    FR_NOT_READY,
    FR_UNSUPPORTED,
    FR_BLOCKED,
    FR_IO,
    FR_EXHAUSTED
};

/* Raw byte operations: valid live extents, nonoverlap for copy, snapshot move,
 * unsigned lexicographic compare sign only. Local API permits null iff n==0.
 * Raw move's implementation is bound to the declared flat-address uintptr_t
 * order; this is not a portable ordering of unrelated ISO C pointers. */
void *fr_copy(void *, const void *, size_t);
void *fr_move(void *, const void *, size_t);
void *fr_set(void *, unsigned char, size_t);
int fr_compare(const void *, const void *, size_t);
/* Portable same-arena offsets: validate before writes; count0 permits end
 * offsets and null arena iff capacity0. Copy rejects intersecting nonempty
 * ranges. Output is unchanged for any admission failure. */
enum fr_status fr_arena_copy(unsigned char *, size_t, size_t, size_t, size_t);
enum fr_status fr_arena_move(unsigned char *, size_t, size_t, size_t, size_t);

struct fr_u128
{
    uint32_t limb[4];
};
/* Limb0 is least significant. q and r must be nonnull, distinct, disjoint from
 * operands. On zero divisor both outputs are unchanged. All nonzero divisors
 * require q*d+r==n and r<d in mathematical integer arithmetic. */
enum fr_status fr_divmod128(struct fr_u128, struct fr_u128, struct fr_u128 *,
                            struct fr_u128 *);
/* Streaming extension: r<divisor, bit0/1; compute (2*r+bit) divmod divisor.
 * This can require a129th transient bit, unlike a zero-initialized division of
 * one128-bit numerator. On error outputs are unchanged and must be disjoint. */
enum fr_status fr_divstep128(struct fr_u128, struct fr_u128, unsigned,
                             struct fr_u128 *, unsigned *);
#endif
