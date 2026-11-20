/*
 * Dispatch-predictability workload: names, selectors, the repetition loop
 * and the independent oracle. Contract: dispatch.h.
 */
#include "dispatch.h"

#include <string.h>

#define DP_SEED UINT64_C(0x2545F4914F6CDD1D)

static const char *const mechanism_names[DP_MECHANISM_COUNT] = {
    [DP_BRANCH] = "branch",
    [DP_INDIRECT] = "indirect",
    [DP_HINTED] = "hinted",
    [DP_BRANCHLESS] = "branchless",
};

static const char *const distribution_names[DP_DISTRIBUTION_COUNT] = {
    [DP_CONSTANT] = "constant",
    [DP_ALTERNATING] = "alternating",
    [DP_RANDOM] = "random",
};

static bool lookup(const char *name, const char *const *names, unsigned count,
                   unsigned *index)
{
    for (unsigned i = 0; (name != NULL) && (i < count); ++i)
    {
        if (strcmp(name, names[i]) == 0)
        {
            *index = i;
            return true;
        }
    }
    return false;
}

bool dp_mechanism_lookup(const char *name, enum dp_mechanism *mechanism)
{
    unsigned index = 0;

    if (!lookup(name, mechanism_names, DP_MECHANISM_COUNT, &index))
    {
        return false;
    }
    *mechanism = (enum dp_mechanism)index;
    return true;
}

bool dp_distribution_lookup(const char *name,
                            enum dp_distribution *distribution)
{
    unsigned index = 0;

    if (!lookup(name, distribution_names, DP_DISTRIBUTION_COUNT, &index))
    {
        return false;
    }
    *distribution = (enum dp_distribution)index;
    return true;
}

const char *dp_mechanism_name(enum dp_mechanism mechanism)
{
    return ((unsigned)mechanism < DP_MECHANISM_COUNT)
               ? mechanism_names[mechanism]
               : "invalid";
}

const char *dp_distribution_name(enum dp_distribution distribution)
{
    return ((unsigned)distribution < DP_DISTRIBUTION_COUNT)
               ? distribution_names[distribution]
               : "invalid";
}

struct dp_selector dp_selector_for(enum dp_distribution distribution)
{
    struct dp_selector selector = {.seed = DP_SEED};

    selector.random_mask = (distribution == DP_RANDOM) ? 1u : 0u;
    selector.alternate_mask = (distribution == DP_ALTERNATING) ? 1u : 0u;
    return selector;
}

struct dp_table dp_table_make(void)
{
    return (struct dp_table){.operation = {dp_operation0, dp_operation1}};
}

uint64_t dp_run(enum dp_mechanism mechanism, const struct dp_selector *selector,
                const struct dp_table *table, size_t n, unsigned repetitions)
{
    uint64_t total = 0;

    for (unsigned r = 0; r < repetitions; ++r)
    {
        switch (mechanism)
        {
            case DP_BRANCH:
                total += dp_kernel_branch(selector, n, r);
                break;
            case DP_INDIRECT:
                total += dp_kernel_indirect(selector, table, n, r);
                break;
            case DP_HINTED:
                total += dp_kernel_hinted(selector, table, n, r);
                break;
            case DP_BRANCHLESS:
            default:
                total += dp_kernel_branchless(selector, n, r);
                break;
        }
    }
    return total;
}

/* The oracle's own generator, written apart from the kernels' helpers:
 * the 64-bit splitmix finalizer, then xorshift64 (13, 7, 17). */
static uint64_t oracle_seed(uint64_t seed, unsigned repeat)
{
    uint64_t z = seed + (uint64_t)repeat + UINT64_C(0x9E3779B97F4A7C15);

    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    z = z ^ (z >> 31);
    return (z == 0u) ? 1u : z;
}

uint64_t dp_oracle_selected(enum dp_distribution distribution, size_t n,
                            unsigned repeat)
{
    if (distribution == DP_CONSTANT)
    {
        return 0;
    }
    if (distribution == DP_ALTERNATING)
    {
        return (uint64_t)n / 2u; /* odd call indices */
    }
    uint64_t state = oracle_seed(DP_SEED, repeat);
    uint64_t selected = 0;
    for (size_t i = 0; i < n; ++i)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        selected += state >> 63;
    }
    return selected;
}

uint64_t dp_oracle(enum dp_distribution distribution, size_t n,
                   unsigned repetitions)
{
    uint64_t count = (uint64_t)n;
    /* Sum of i for i < n, halving the even factor so nothing is lost
     * before the modular product. */
    uint64_t indices = ((count % 2u) == 0u) ? (count / 2u) * (count - 1u)
                                            : count * ((count - 1u) / 2u);
    uint64_t total = 0;

    for (unsigned r = 0; r < repetitions; ++r)
    {
        uint64_t values = indices + count * r; /* sum of i + r */

        total += 33u * values + 17u * count +
                 2u * dp_oracle_selected(distribution, n, r);
    }
    return total;
}
