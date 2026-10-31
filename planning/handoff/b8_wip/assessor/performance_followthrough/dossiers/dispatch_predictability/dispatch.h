/*
 * Dispatch-predictability dossier workload. Author-only: it answers the
 * changed condition of an assessed T43 task ("change branch predictability
 * while keeping semantic work fixed"), so it is not learner content.
 * Derived from the dispatch family of labs/performance_followthrough
 * (pmu_workloads.c: operation0/operation1 chosen by a branch or by an
 * indirect call through a runtime table).
 *
 * Each of n calls computes value = i + repeat, draws a selector bit and
 * applies operation0 (value * 33 + 17) or operation1 (value * 33 + 19):
 *
 *   branch      if (select) operation1(value) else operation0(value):
 *               a data-dependent conditional branch to two direct calls
 *   indirect    table->operation[select](value): an indirect call through
 *               a table filled at run time
 *   hinted      the same indirect call, preceded by a conditional branch on
 *               the same selector bit that stores to a volatile sink: the
 *               branch's direction enters the global branch history before
 *               the call is predicted
 *   branchless  value * 33 + 17 + 2 * select: no data-dependent control
 *               flow and no call
 *
 * The selector distribution is data, not code. Every mechanism runs the
 * same xorshift64 generator on every call; a pair of masks decides whether
 * its bit is used:
 *
 *   constant     select = 0 on every call
 *   alternating  select = i & 1 (period two)
 *   random       select = top bit of the generator (fresh per repetition)
 *
 * so a mechanism executes identical instructions under every distribution
 * and only the selector values change.
 *
 * Correctness contract (tests.c, before any measurement): for every
 * mechanism, distribution, n and repetitions, the sum equals dp_oracle,
 * which is computed by a separately written generator and a closed form.
 */
#ifndef DP_DISPATCH_H
#define DP_DISPATCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DP_MAX_N (UINT64_C(1) << 24)

enum dp_mechanism
{
    DP_BRANCH = 0,
    DP_INDIRECT,
    DP_HINTED,
    DP_BRANCHLESS,
    DP_MECHANISM_COUNT
};

enum dp_distribution
{
    DP_CONSTANT = 0,
    DP_ALTERNATING,
    DP_RANDOM,
    DP_DISTRIBUTION_COUNT
};

struct dp_selector
{
    uint64_t random_mask; /* 1: use the generator's top bit */
    uint64_t alternate_mask; /* 1: use the call index's low bit */
    uint64_t seed; /* nonzero generator seed for repetition 0 */
};

struct dp_table
{
    uint64_t (*operation[2])(uint64_t value);
};

bool dp_mechanism_lookup(const char *name, enum dp_mechanism *mechanism);
bool dp_distribution_lookup(const char *name,
                            enum dp_distribution *distribution);
const char *dp_mechanism_name(enum dp_mechanism mechanism);
const char *dp_distribution_name(enum dp_distribution distribution);

/* The selector for `distribution` with the fixed dossier seed. */
struct dp_selector dp_selector_for(enum dp_distribution distribution);

/* The run-time table {dp_operation0, dp_operation1}. */
struct dp_table dp_table_make(void);

/* Sum of `repetitions` passes of n calls with `mechanism`. n is 1..MAX. */
uint64_t dp_run(enum dp_mechanism mechanism, const struct dp_selector *selector,
                const struct dp_table *table, size_t n, unsigned repetitions);

/* Number of calls that selected operation1 in repetition `repeat`, by a
 * separately written generator. */
uint64_t dp_oracle_selected(enum dp_distribution distribution, size_t n,
                            unsigned repeat);

/* Independent prediction of dp_run's result for this distribution. */
uint64_t dp_oracle(enum dp_distribution distribution, size_t n,
                   unsigned repetitions);

/* The measured kernels (one repetition each) and the two operations, in
 * dispatch_kernels.c. The operations are noinline (GNU attribute) so the
 * branch and indirect kernels keep real calls. */
uint64_t dp_operation0(uint64_t value);
uint64_t dp_operation1(uint64_t value);
uint64_t dp_kernel_branch(const struct dp_selector *selector, size_t n,
                          unsigned repeat);
uint64_t dp_kernel_indirect(const struct dp_selector *selector,
                            const struct dp_table *table, size_t n,
                            unsigned repeat);
uint64_t dp_kernel_hinted(const struct dp_selector *selector,
                          const struct dp_table *table, size_t n,
                          unsigned repeat);
uint64_t dp_kernel_branchless(const struct dp_selector *selector, size_t n,
                              unsigned repeat);

#endif /* DP_DISPATCH_H */
