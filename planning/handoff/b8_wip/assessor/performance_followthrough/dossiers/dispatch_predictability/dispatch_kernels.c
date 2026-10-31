/*
 * The measured kernels and operations of the dispatch workload
 * (dispatch.h). One translation unit, compiled without link-time
 * optimization, so the driver's repetition loop sees only external calls.
 *
 * The four kernels share one selector step (next_select), so under every
 * distribution a kernel's instruction stream is the same; only the values
 * of `select`, and hence branch directions or call targets, change.
 */
#include "dispatch.h"

/* GNU attribute: keep real calls to compare with the indirect call. */
__attribute__((noinline)) uint64_t dp_operation0(uint64_t value)
{
    return value * 33u + 17u;
}

__attribute__((noinline)) uint64_t dp_operation1(uint64_t value)
{
    return value * 33u + 19u;
}

/* splitmix64(seed + repeat), never zero: xorshift64 has no zero state. */
static uint64_t first_state(const struct dp_selector *selector, unsigned repeat)
{
    uint64_t x = selector->seed + repeat;

    x += UINT64_C(0x9E3779B97F4A7C15);
    x = (x ^ (x >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94D049BB133111EB);
    x ^= x >> 31;
    return (x != 0u) ? x : 1u;
}

/* One xorshift64 step, then the distribution's selector bit. */
static inline uint64_t next_select(uint64_t *state, uint64_t random_mask,
                                   uint64_t alternate_mask, size_t i)
{
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return ((x >> 63) & random_mask) ^ ((uint64_t)i & alternate_mask);
}

uint64_t dp_kernel_branch(const struct dp_selector *selector, size_t n,
                          unsigned repeat)
{
    uint64_t random_mask = selector->random_mask;
    uint64_t alternate_mask = selector->alternate_mask;
    uint64_t state = first_state(selector, repeat);
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        uint64_t value = (uint64_t)i + repeat;

        if (next_select(&state, random_mask, alternate_mask, i) != 0u)
        {
            sum += dp_operation1(value);
        }
        else
        {
            sum += dp_operation0(value);
        }
    }
    return sum;
}

uint64_t dp_kernel_indirect(const struct dp_selector *selector,
                            const struct dp_table *table, size_t n,
                            unsigned repeat)
{
    uint64_t random_mask = selector->random_mask;
    uint64_t alternate_mask = selector->alternate_mask;
    uint64_t state = first_state(selector, repeat);
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        uint64_t value = (uint64_t)i + repeat;
        uint64_t select = next_select(&state, random_mask, alternate_mask, i);

        sum += table->operation[select](value);
    }
    return sum;
}

/* Written only on calls that select operation1: a store the compiler must
 * keep conditional, so the selector bit reaches the branch history as the
 * direction of a real conditional branch. */
static volatile uint64_t hint_sink;

uint64_t dp_kernel_hinted(const struct dp_selector *selector,
                          const struct dp_table *table, size_t n,
                          unsigned repeat)
{
    uint64_t random_mask = selector->random_mask;
    uint64_t alternate_mask = selector->alternate_mask;
    uint64_t state = first_state(selector, repeat);
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        uint64_t value = (uint64_t)i + repeat;
        uint64_t select = next_select(&state, random_mask, alternate_mask, i);

        if (select != 0u)
        {
            hint_sink = value;
        }
        sum += table->operation[select](value);
    }
    return sum;
}

uint64_t dp_kernel_branchless(const struct dp_selector *selector, size_t n,
                              unsigned repeat)
{
    uint64_t random_mask = selector->random_mask;
    uint64_t alternate_mask = selector->alternate_mask;
    uint64_t state = first_state(selector, repeat);
    uint64_t sum = 0;

    for (size_t i = 0; i < n; ++i)
    {
        uint64_t value = (uint64_t)i + repeat;
        uint64_t select = next_select(&state, random_mask, alternate_mask, i);

        sum += value * 33u + 17u + (select << 1);
    }
    return sum;
}
