/*
 * Deterministic contract tests for the dispatch workload (dispatch.h), run
 * before any measurement. The expected sums come from a direct simulation
 * written here from the documented generator; dp_oracle and every
 * mechanism are compared with it. The distributions themselves are
 * checked: their selection counts, switch counts and per-repetition
 * freshness are what make them constant, alternating or random.
 *
 * Output: DP_TEST_PASS; DP_TEST_ASSERT:<case> on stderr and exit 1.
 */
#include "dispatch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(bool condition, const char *what)
{
    if (!condition)
    {
        (void)fprintf(stderr, "DP_TEST_ASSERT:%s\n", what);
        exit(1);
    }
}

/* The documented generator: splitmix64 of seed + repeat (1 if that is
 * zero) seeds xorshift64 (13, 7, 17); each call steps it once. */
static uint64_t splitmix64(uint64_t x)
{
    x += UINT64_C(0x9E3779B97F4A7C15);
    x = (x ^ (x >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94D049BB133111EB);
    return x ^ (x >> 31);
}

static void simulate_selects(const struct dp_selector *selector, size_t n,
                             unsigned repeat, unsigned char *selects)
{
    uint64_t state = splitmix64(selector->seed + repeat);

    if (state == 0u)
    {
        state = 1u;
    }
    for (size_t i = 0; i < n; ++i)
    {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        selects[i] = (unsigned char)(((state >> 63) & selector->random_mask) ^
                                     ((uint64_t)i & selector->alternate_mask));
    }
}

static uint64_t simulate_sum(const struct dp_selector *selector, size_t n,
                             unsigned repetitions, unsigned char *scratch)
{
    uint64_t total = 0;

    for (unsigned r = 0; r < repetitions; ++r)
    {
        simulate_selects(selector, n, r, scratch);
        for (size_t i = 0; i < n; ++i)
        {
            uint64_t value = (uint64_t)i + r;

            total += value * 33u + (scratch[i] ? 19u : 17u);
        }
    }
    return total;
}

static void test_names(void)
{
    static const char *const mechanisms[DP_MECHANISM_COUNT] = {
        "branch", "indirect", "hinted", "branchless"};
    static const char *const distributions[DP_DISTRIBUTION_COUNT] = {
        "constant", "alternating", "random"};
    enum dp_mechanism mechanism = DP_MECHANISM_COUNT;
    enum dp_distribution distribution = DP_DISTRIBUTION_COUNT;

    for (unsigned i = 0; i < DP_MECHANISM_COUNT; ++i)
    {
        require(dp_mechanism_lookup(mechanisms[i], &mechanism) &&
                    (mechanism == (enum dp_mechanism)i) &&
                    (strcmp(dp_mechanism_name(mechanism), mechanisms[i]) == 0),
                "mechanism names");
    }
    for (unsigned i = 0; i < DP_DISTRIBUTION_COUNT; ++i)
    {
        require(dp_distribution_lookup(distributions[i], &distribution) &&
                    (distribution == (enum dp_distribution)i) &&
                    (strcmp(dp_distribution_name(distribution),
                            distributions[i]) == 0),
                "distribution names");
    }
    mechanism = DP_INDIRECT;
    distribution = DP_RANDOM;
    require(!dp_mechanism_lookup("Branch", &mechanism) &&
                !dp_mechanism_lookup("", &mechanism) &&
                !dp_mechanism_lookup(NULL, &mechanism) &&
                !dp_distribution_lookup("randomly", &distribution) &&
                !dp_distribution_lookup(NULL, &distribution),
            "unknown names rejected");
    require((mechanism == DP_INDIRECT) && (distribution == DP_RANDOM),
            "rejected lookup leaves output");
}

static void test_selectors(void)
{
    struct dp_selector constant = dp_selector_for(DP_CONSTANT);
    struct dp_selector alternating = dp_selector_for(DP_ALTERNATING);
    struct dp_selector random = dp_selector_for(DP_RANDOM);
    struct dp_table table = dp_table_make();

    require((constant.random_mask == 0u) && (constant.alternate_mask == 0u),
            "constant masks");
    require((alternating.random_mask == 0u) &&
                (alternating.alternate_mask == 1u),
            "alternating masks");
    require((random.random_mask == 1u) && (random.alternate_mask == 0u),
            "random masks");
    require((constant.seed == random.seed) && (random.seed != 0u),
            "one nonzero seed for every distribution");
    require((table.operation[0] == dp_operation0) &&
                (table.operation[1] == dp_operation1),
            "run-time table");
    require((dp_operation0(5) == 182u) && (dp_operation1(5) == 184u),
            "operations");
}

/* Count selections, switches and repetition-to-repetition differences. */
static void test_distributions(unsigned char *a, unsigned char *b)
{
    const size_t n = 65536;
    uint64_t ones[DP_DISTRIBUTION_COUNT] = {0};
    uint64_t switches[DP_DISTRIBUTION_COUNT] = {0};
    uint64_t fresh[DP_DISTRIBUTION_COUNT] = {0};

    for (unsigned d = 0; d < DP_DISTRIBUTION_COUNT; ++d)
    {
        struct dp_selector selector = dp_selector_for((enum dp_distribution)d);

        simulate_selects(&selector, n, 0, a);
        simulate_selects(&selector, n, 1, b);
        for (size_t i = 0; i < n; ++i)
        {
            ones[d] += a[i];
            switches[d] += (i > 0u) && (a[i] != a[i - 1u]);
            fresh[d] += (a[i] != b[i]);
        }
        require(dp_oracle_selected((enum dp_distribution)d, n, 0) == ones[d],
                "oracle selection count");
    }
    require((ones[DP_CONSTANT] == 0u) && (switches[DP_CONSTANT] == 0u),
            "constant never selects operation1");
    require((ones[DP_ALTERNATING] == n / 2u) &&
                (switches[DP_ALTERNATING] == n - 1u) &&
                (fresh[DP_ALTERNATING] == 0u),
            "alternating has period two and repeats exactly");
    require((ones[DP_RANDOM] > 32112u) && (ones[DP_RANDOM] < 33424u),
            "random selects about half the calls");
    require((switches[DP_RANDOM] > 32112u) && (switches[DP_RANDOM] < 33424u),
            "random switches on about half the calls");
    require((fresh[DP_RANDOM] > 32112u) && (fresh[DP_RANDOM] < 33424u),
            "random sequence is fresh in every repetition");
}

static void test_sums(unsigned char *scratch)
{
    static const size_t sizes[] = {1, 2, 3, 17, 1000, 65536};
    static const unsigned repetitions[] = {1, 2, 3};
    struct dp_table table = dp_table_make();

    for (unsigned d = 0; d < DP_DISTRIBUTION_COUNT; ++d)
    {
        struct dp_selector selector = dp_selector_for((enum dp_distribution)d);

        for (size_t s = 0; s < sizeof sizes / sizeof sizes[0]; ++s)
        {
            for (size_t r = 0; r < sizeof repetitions / sizeof repetitions[0];
                 ++r)
            {
                uint64_t expected =
                    simulate_sum(&selector, sizes[s], repetitions[r], scratch);

                require(dp_oracle((enum dp_distribution)d, sizes[s],
                                  repetitions[r]) == expected,
                        "oracle equals the simulation");
                for (unsigned m = 0; m < DP_MECHANISM_COUNT; ++m)
                {
                    require(dp_run((enum dp_mechanism)m, &selector, &table,
                                   sizes[s], repetitions[r]) == expected,
                            "mechanism equals the simulation");
                }
            }
        }
    }
}

int main(void)
{
    unsigned char *a = malloc(65536);
    unsigned char *b = malloc(65536);

    require((a != NULL) && (b != NULL), "scratch");
    test_names();
    test_selectors();
    test_distributions(a, b);
    test_sums(a);
    free(a);
    free(b);
    (void)puts("DP_TEST_PASS");
    return 0;
}
