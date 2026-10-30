#ifndef FE_ENTRY_H
#define FE_ENTRY_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>

/* The traced functions accept at most FE_TRACE_MAX_ELEMENTS values. A trace
 * records one entering and one returning event for each of the n + 1
 * activations (the base case included): 2 * (16 + 1) = 34 events. */
enum
{
    FE_TRACE_MAX_ELEMENTS = 16,
    FE_TRACE_EVENT_CAPACITY = 2 * (FE_TRACE_MAX_ELEMENTS + 1)
};

/* Status of every fe_* function that returns one. Unless stated otherwise a
 * failure leaves the caller's result object unchanged. */
enum fe_status
{
    FE_OK,
    /* Null pointer where an object is required. */
    FE_INVALID,
    /* A signed addition would leave the range of int32_t, or a value does not
     * fit the encoding. */
    FE_RANGE,
    /* Input longer than the function's capacity or output too small. */
    FE_FULL,
    /* The allocator callback returned null. */
    FE_ALLOC,
    /* No elements, so there is no maximum. */
    FE_EMPTY,
    /* Returned by the unfinished learner stubs. */
    FE_NOT_IMPLEMENTED
};

/* One step of the recursive evaluation S(i) = input[i] + S(i + 1). */
struct fe_event
{
    /* 1 when activation index is entered (value is then 0); 0 when it
     * returns the suffix sum in value. */
    unsigned entering;
    size_t index;
    int32_t value;
};

/* Observation output of fe_recursive() and fe_explicit_stack(). */
struct fe_trace
{
    struct fe_event event[FE_TRACE_EVENT_CAPACITY];
    /* Number of valid entries in event. */
    size_t count;
    /* Most activations suspended at once, base case included. */
    size_t maximum_live;
};

/* Allocation callbacks for fe_staged(). take(context, n) returns null or a
 * live array of at least n int32_t objects disjoint from the caller's arrays;
 * release(context, block) gives back exactly that block. The callbacks must
 * return normally and must not modify the input. */
struct fe_allocator
{
    void *context;
    int32_t *(*take)(void *context, size_t count);
    void (*release)(void *context, int32_t *block);
};

/* Common contract of the prefix-sum functions (output[i] is the sum of
 * input[0..i]): every pointed-to control object is live and disjoint. After
 * scalar admission, count must not exceed the actual accessible extent of the
 * arrays (C cannot check that), and a size check rejects before any access
 * or allocation. Each running sum is checked before it is formed, and a
 * range or allocation failure preserves the output. Input and output are
 * borrowed for the call only. None of these functions blocks. Null arrays
 * are accepted when the count is 0.
 *
 * Trace outputs: an admitted call resets the trace and keeps a partial
 * history when it fails midway. A call rejected before admission leaves the
 * old trace alone. */

/* Calls the supplied f_prefix: at most 16 values, any input/output overlap.
 * FE_FULL when count > 16 or count > capacity, FE_RANGE on overflow, and
 * FE_INVALID for every other f_prefix failure. */
enum fe_status fe_fixed(const int32_t *input, size_t count, int32_t *output,
                        size_t capacity);

/* Recursive total of at most 16 values, traced. Publishes the total through
 * total only on success. FE_INVALID for null total or trace, or a null input
 * with count > 0; FE_FULL when count > 16; FE_RANGE on overflow. */
enum fe_status fe_recursive(const int32_t *input, size_t count, int32_t *total,
                            struct fe_trace *trace);

/* The same total and trace as fe_recursive(), computed with an explicit stack
 * of one record per suspended activation and no C recursion. Same failures. */
enum fe_status fe_explicit_stack(const int32_t *input, size_t count,
                                 int32_t *total, struct fe_trace *trace);

/* Prefix sums of any length: all prefixes are checked first, then the output
 * is written. Input and output must be disjoint or start at the same
 * address; partial overlap is outside the contract. FE_FULL when
 * count > capacity, FE_INVALID for a null array with count > 0, FE_RANGE on
 * overflow (output unchanged). */
enum fe_status fe_two_pass(const int32_t *input, size_t count, int32_t *output,
                           size_t capacity);

/* Prefix sums of any length with arbitrary input/output overlap, computed in
 * storage obtained from allocator and copied out only after every prefix
 * passed. A non-empty request takes exactly one block and releases it exactly
 * once, also when an addition fails; an empty request allocates nothing.
 * FE_RANGE when count > SIZE_MAX / sizeof(int32_t) (before any allocation)
 * or on overflow; FE_INVALID for a missing callback; FE_ALLOC when take
 * returns null; plus the fe_two_pass() failures. */
enum fe_status fe_staged(const int32_t *input, size_t count, int32_t *output,
                         size_t capacity, struct fe_allocator allocator);

/* Greatest of count values. FE_EMPTY (output unchanged) for no values,
 * FE_INVALID for a null output or a null input with count > 0. */
enum fe_status fe_maximum(const int32_t *input, size_t count, int32_t *maximum);

/* Writes value as exactly three octets, most significant first.
 * Checks, in order: output non-null (FE_INVALID), capacity_bytes >= 3
 * (FE_FULL), value <= 0xffffff (FE_RANGE). Storage is untouched on failure
 * and bytes past the third are never written. */
enum fe_status fe_encode24(uint32_t value, unsigned char *output,
                           size_t capacity_bytes);

/* Each returns exactly 1 or 0 for bits and mask.
 * fe_any: some selected bit is set. fe_all: every selected bit is set (true
 * for an empty mask). fe_forbidden_clear: no bit of the forbidden mask is
 * set (true for an empty mask). */
int fe_any(uint8_t bits, uint8_t mask);
int fe_all(uint8_t bits, uint8_t mask);
int fe_forbidden_clear(uint8_t bits, uint8_t forbidden);

/* Mathematical a + b clamped to [INT32_MIN, INT32_MAX]. Saturation is a
 * specified policy; the function never executes a signed overflow. */
int32_t fe_saturating_add(int32_t a, int32_t b);
#endif
