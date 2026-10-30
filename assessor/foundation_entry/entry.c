/* Reference implementation of the entry.h contract. Assessor-only.
 *
 * FE_MUTANT selects a wrong-but-defined variant for the mutation campaign
 * (0 is the correct reference); no mutant executes undefined behaviour. */
#include "entry.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX, "explicit entry profile");
#ifndef FE_MUTANT
#define FE_MUTANT 0
#endif

/* Largest value fe_encode24() accepts, and the octets it writes. */
enum
{
    ENCODE24_OCTETS = 3,
    OCTET_BITS = 8
};

#define ENCODE24_MAX UINT32_C(0xffffff)

/* Values of struct fe_event.entering. */
enum
{
    EVENT_RETURNING = 0,
    EVENT_ENTERING = 1
};

/* Computes a + b without executing a signed overflow.
 * Returns 1 and stores the sum in *sum, or returns 0 and leaves *sum
 * unchanged when the true sum is outside the range of int32_t. */
static int add_checked(int32_t a, int32_t b, int32_t *sum)
{
    const int overflows_up = b > 0 && a > INT32_MAX - b;
    const int overflows_down = b < 0 && a < INT32_MIN - b;

    if (overflows_up || overflows_down)
    {
        return 0;
    }
    *sum = a + b;
    return 1;
}

enum fe_status fe_fixed(const int32_t *input, size_t count, int32_t *output,
                        size_t capacity)
{
    switch (f_prefix(input, count, output, capacity))
    {
        case F_OK:
            return FE_OK;
        case F_FULL:
            return FE_FULL;
        case F_RANGE:
            return FE_RANGE;
        default:
            return FE_INVALID;
    }
}

/* Appends one event. Capacity holds because admission limits count to
 * FE_TRACE_MAX_ELEMENTS and each of the count + 1 activations adds exactly
 * one entering and at most one returning event. */
static void trace_event(struct fe_trace *trace, unsigned entering, size_t index,
                        int32_t value)
{
    trace->event[trace->count++] = (struct fe_event){entering, index, value};
}

/* One activation of S(index) = input[index] + S(index + 1), with S(count) = 0.
 * Publishes the suffix sum through *suffix only on FE_OK. A failing
 * activation records its entry but no return event, so the trace shows the
 * unwinding that did not happen. Recursion depth is at most count + 1, which
 * the caller has bounded by FE_TRACE_MAX_ELEMENTS + 1. */
static enum fe_status suffix_sum(const int32_t *input, size_t count,
                                 size_t index, int32_t *suffix,
                                 struct fe_trace *trace)
{
    trace_event(trace, EVENT_ENTERING, index, 0);
    if (trace->maximum_live < index + 1)
    {
        trace->maximum_live = index + 1;
    }

    int32_t value = 0;
    if (index != count)
    {
        enum fe_status status =
            suffix_sum(input, count, index + 1, &value, trace);
        if (status != FE_OK)
        {
            return status;
        }
        if (!add_checked(input[index], value, &value))
        {
            return FE_RANGE;
        }
    }
    trace_event(trace, EVENT_RETURNING, index, value);
    *suffix = value;
    return FE_OK;
}

/* Admission shared by both traced functions. On FE_OK the trace is reset;
 * on any failure it is left as the caller had it. */
static enum fe_status trace_admit(const int32_t *input, size_t count,
                                  int32_t *total, struct fe_trace *trace)
{
    if (total == NULL || trace == NULL || (count > 0 && input == NULL))
    {
        return FE_INVALID;
    }
    if (count > FE_TRACE_MAX_ELEMENTS)
    {
        return FE_FULL;
    }
    trace->count = 0;
    trace->maximum_live = 0;
    return FE_OK;
}

enum fe_status fe_recursive(const int32_t *input, size_t count, int32_t *total,
                            struct fe_trace *trace)
{
    enum fe_status status = trace_admit(input, count, total, trace);
    if (status != FE_OK)
    {
        return status;
    }

    int32_t value;
    status = suffix_sum(input, count, 0, &value, trace);
    if (status == FE_OK)
    {
        *total = value;
    }
    return status;
}

enum fe_status fe_explicit_stack(const int32_t *input, size_t count,
                                 int32_t *total, struct fe_trace *trace)
{
    enum fe_status status = trace_admit(input, count, total, trace);
    if (status != FE_OK)
    {
        return status;
    }

    /* One record for each suspended recursive activation, including the
     * base case: a count of at most FE_TRACE_MAX_ELEMENTS gives count + 1
     * records. */
    struct suspended_activation
    {
        size_t index;
    } frames[FE_TRACE_MAX_ELEMENTS + 1];

    size_t live = 0;

    /* Descent: enter activations 0..count, as the recursion does before any
     * of them returns. */
    for (size_t index = 0; index <= count; ++index)
    {
        frames[live++].index = index;
        trace_event(trace, EVENT_ENTERING, index, 0);
    }
    trace->maximum_live = live;

    /* Unwinding: pop the innermost activation, add its element to the
     * suffix sum of the one above it, and record its return. */
    int32_t value = 0;
    while (live > 0)
    {
        const size_t index = frames[--live].index;

        if (index != count && !add_checked(input[index], value, &value))
        {
            return FE_RANGE;
        }
        if (FE_MUTANT != 1 || index != count)
        {
            trace_event(trace, EVENT_RETURNING, index, value);
        }
    }
    *total = value;
    return FE_OK;
}

/* Scalar admission shared by the unbounded prefix functions: count against
 * capacity first, then the pointers needed for a non-empty request. */
static enum fe_status check_extent(const int32_t *input, size_t count,
                                   int32_t *output, size_t capacity)
{
    if (count > capacity)
    {
        return FE_FULL;
    }
    if (count > 0 && (input == NULL || output == NULL))
    {
        return FE_INVALID;
    }
    return FE_OK;
}

enum fe_status fe_two_pass(const int32_t *input, size_t count, int32_t *output,
                           size_t capacity)
{
    enum fe_status status = check_extent(input, count, output, capacity);
    if (status != FE_OK)
    {
        return status;
    }

    /* Pass 1 only reads: it proves every prefix fits before output changes. */
    int32_t total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (!add_checked(total, input[i], &total))
        {
            return FE_RANGE;
        }
        if (FE_MUTANT == 2)
        {
            output[i] = total;
        }
    }

    /* Pass 2 may write in place: output[i] is stored only after input[i] was
     * read, and a later input element is never an earlier output element
     * (disjoint arrays or identical start addresses). */
    total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        total += input[i]; /* Cannot overflow: pass 1 checked this addition. */
        output[i] = total;
    }
    return FE_OK;
}

enum fe_status fe_staged(const int32_t *input, size_t count, int32_t *output,
                         size_t capacity, struct fe_allocator allocator)
{
    enum fe_status status = check_extent(input, count, output, capacity);
    if (status != FE_OK || count == 0)
    {
        return status;
    }
    if (count > SIZE_MAX / sizeof(int32_t))
    {
        return FE_RANGE;
    }
    if (allocator.take == NULL || allocator.release == NULL)
    {
        return FE_INVALID;
    }

    /* From here the stage block is owned by this function and must be
     * released exactly once on every path. The output is written only after
     * all prefixes are computed, so overlap with the input is harmless. */
    int32_t *stage = allocator.take(allocator.context, count);
    if (stage == NULL)
    {
        return FE_ALLOC;
    }
    int32_t total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (!add_checked(total, input[i], &total))
        {
            if (FE_MUTANT != 3)
            {
                allocator.release(allocator.context, stage);
            }
            return FE_RANGE;
        }
        stage[i] = total;
    }
    for (size_t i = 0; i < count; ++i)
    {
        output[i] = stage[i];
    }
    allocator.release(allocator.context, stage);
    return FE_OK;
}

enum fe_status fe_maximum(const int32_t *input, size_t count, int32_t *maximum)
{
    if (maximum == NULL || (count > 0 && input == NULL))
    {
        return FE_INVALID;
    }
    if (count == 0)
    {
        if (FE_MUTANT == 4)
        {
            *maximum = 0;
        }
        return FE_EMPTY;
    }

    int32_t largest = input[0];
    for (size_t i = 1; i < count; ++i)
    {
        if (input[i] > largest)
        {
            largest = input[i];
        }
    }
    *maximum = largest;
    return FE_OK;
}

enum fe_status fe_encode24(uint32_t value, unsigned char *output,
                           size_t capacity_bytes)
{
    if (output == NULL)
    {
        return FE_INVALID;
    }
    if (capacity_bytes < ENCODE24_OCTETS)
    {
        return FE_FULL;
    }
    if (value > ENCODE24_MAX)
    {
        return FE_RANGE;
    }

    /* Octet i of the wire format holds bits [8 * (2 - i), 8 * (3 - i)) of
     * the value: most significant first, whatever the host byte order.
     * Mutant 5 writes the octets in the opposite order. */
    for (unsigned i = 0; i < ENCODE24_OCTETS; ++i)
    {
        const unsigned octets_below =
            FE_MUTANT == 5 ? i : ENCODE24_OCTETS - 1u - i;
        output[i] = (unsigned char)(value >> (octets_below * OCTET_BITS));
    }
    return FE_OK;
}

int fe_any(uint8_t bits, uint8_t mask)
{
    return (bits & mask) != 0;
}

int fe_all(uint8_t bits, uint8_t mask)
{
    return (bits & mask) == mask;
}

int fe_forbidden_clear(uint8_t bits, uint8_t forbidden)
{
    if (FE_MUTANT == 6)
    {
        return fe_all(bits, forbidden);
    }
    return (bits & forbidden) == 0;
}

int32_t fe_saturating_add(int32_t a, int32_t b)
{
    int32_t sum;

    if (add_checked(a, b, &sum))
    {
        return sum;
    }
    /* Overflow is possible only in the direction of b's sign. */
    return b > 0 ? INT32_MAX : INT32_MIN;
}
