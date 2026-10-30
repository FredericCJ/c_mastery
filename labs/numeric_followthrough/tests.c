#define _GNU_SOURCE
#include "numeric.h"
#include "checkpoint_vector.h"
#include <fenv.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "NF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)
void cp_test_numeric_transaction(void);
void cp_test_numeric(void);
void cp_test_numeric_policy(void);
void cp_test_dispatch(void);
void cp_test_vector(void);

/* Guard-page probe geometry.  Lengths 0..GUARD_MAX_LENGTH cover W (4) through
 * 2W+1 for the 4-double AVX width; GUARD_OFFSETS gives one offset per double
 * in a 32-byte period. */
enum
{
    GUARD_BACKENDS = 4,
    GUARD_MAX_LENGTH = 9,
    GUARD_OFFSETS = 4,
    GUARD_MAP_PAGES = 3
};

/* Reserves three pages and makes only the middle one accessible, so a read or
 * write one byte outside it faults.  Returns the base of the reservation. */
static unsigned char *map_with_guards(size_t page_bytes)
{
    unsigned char *base = mmap(0, GUARD_MAP_PAGES * page_bytes, PROT_NONE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(base != MAP_FAILED);
    CHECK(!mprotect(base + page_bytes, page_bytes, PROT_READ | PROT_WRITE));
    return base;
}

/* Side 0 ends the n inputs `offset` doubles before the upper guard page; side
 * 1 starts them `offset` doubles after the lower one.  Both ends are probed. */
static double *guarded_input(unsigned char *map, size_t page_bytes, size_t n,
                             unsigned side, size_t offset)
{
    unsigned char *page = map + (side ? page_bytes : 2 * page_bytes);
    ptrdiff_t first = side ? (ptrdiff_t)offset : -(ptrdiff_t)(n + offset);
    return (double *)(void *)page + first;
}

/* Runs the vector backend on guarded buffers and compares it with the scalar
 * reference: outputs, and both history values, must match exactly. */
static void check_guarded_case(const struct cpv_plan *plan,
                               unsigned char *input_map,
                               unsigned char *output_map, size_t page_bytes,
                               size_t n, unsigned side, size_t offset)
{
    double *in = guarded_input(input_map, page_bytes, n, side, offset);
    /* The output always ends exactly at the upper guard page. */
    double *out = (double *)(void *)(output_map + 2 * page_bytes) - n;
    for (size_t i = 0; i < n; i++)
    {
        in[i] = (double)((int)i - 3) / 8;
    }

    double expected[GUARD_MAX_LENGTH];
    struct cpn_double reference = {4, -8};
    struct cpn_double actual = reference;
    CHECK(cpn_double(&reference, in, n, expected, n) == CPN_OK);
    CHECK(cpv_run(plan, &actual, in, n, out, n) == CPN_OK);
    CHECK(!memcmp(expected, out, n * sizeof(double)));
    CHECK(actual.previous == reference.previous);
    CHECK(actual.older == reference.older);
}

/* Reports the number of guarded executions in the NF_GUARDS marker. */
static void guards(void)
{
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0 && (size_t)page >= (CPN_BLOCK + 4) * sizeof(double));
    const size_t page_bytes = (size_t)page;
    unsigned char *input_map = map_with_guards(page_bytes);
    unsigned char *output_map = map_with_guards(page_bytes);

    unsigned executed = 0;
    for (unsigned backend = 0; backend < GUARD_BACKENDS; backend++)
    {
        struct cpv_plan plan;
        if (cpv_plan(&plan, (enum cpv_backend)backend) == CPN_UNSUPPORTED)
        {
            continue;
        }
        for (size_t n = 0; n <= GUARD_MAX_LENGTH; n++)
        {
            for (unsigned side = 0; side < 2; side++)
            {
                for (size_t offset = 0; offset < GUARD_OFFSETS; offset++)
                {
                    check_guarded_case(&plan, input_map, output_map, page_bytes,
                                       n, side, offset);
                    executed++;
                }
            }
        }
    }
    CHECK(!munmap(input_map, GUARD_MAP_PAGES * page_bytes));
    CHECK(!munmap(output_map, GUARD_MAP_PAGES * page_bytes));
    printf("NF_GUARDS %u\n", executed);
}

/* nf_compare policy table: telemetry has an absolute floor, audit compares
 * bits (including signed zero), and refusals leave *out unchanged. */
static void policy_decisions(void)
{
    struct nf_error e = {7, 8, 9};
    CHECK(!nf_compare(NF_TELEMETRY, 0, 0x1p-44, &e) && e.accepted &&
          e.absolute == 0x1p-44);
    CHECK(!nf_compare(NF_AUDIT, 0, 0x1p-44, &e) && !e.accepted);
    CHECK(!nf_compare(NF_TELEMETRY, 0, -0.0, &e) && e.accepted);
    CHECK(!nf_compare(NF_AUDIT, 0, -0.0, &e) && !e.accepted);
    CHECK(!nf_compare(NF_TELEMETRY, 1, 0, &e) && !e.accepted &&
          e.absolute == 1);

    const struct nf_error old = e;
    CHECK(nf_compare(NF_TELEMETRY, NAN, 0, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
    CHECK(nf_compare(NF_TELEMETRY, 0, INFINITY, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
    CHECK(nf_compare((enum nf_policy)99, 0, 0, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
}

/* FIR input whose two large terms cancel: the ordered scalar result is 0, the
 * reordered sum is 1 (the rational answer).  Audit must reject the difference. */
static void cancellation_case(void)
{
    const double input[] = {-0x1p55, 0x1p54, 4};
    struct cpn_double state = {0, 0};
    double out[3];
    CHECK(cpn_double(&state, input, 3, out, 3) == CPN_OK);

    volatile double reordered =
        (.5 * input[1] + .25 * input[0]) + .25 * input[2];
    CHECK(out[2] == 0 && reordered == 1);

    struct nf_error e;
    CHECK(!nf_compare(NF_AUDIT, out[2], reordered, &e) && !e.accepted);
    printf("NF_CANCELLATION scalar=%a reordered=%a rational=1 absolute=%a\n",
           out[2], (double)reordered, e.absolute);
}

static void policies(void)
{
    policy_decisions();
    cancellation_case();

    /* The smallest binary64 subnormal survives identity additions here. */
    volatile double tiny = 0x1p-1074;
    volatile double zero = 0;
    CHECK(tiny + zero == tiny);
    CHECK(fegetround() == FE_TONEAREST);
    puts("NF_POLICY PASS telemetry-absolute-plus-relative audit-bits");
}

/* Corpus sizes are fixed because the producer's oracle checks the counts. */
enum
{
    CORPUS_CONVOLUTIONS = 4096,
    CORPUS_WINDOW = 5,
    CORPUS_ROUND_LIMIT = 10001
};

/* Numerical Recipes linear congruential step; deterministic, not random. */
static uint32_t next_lcg(uint32_t state)
{
    return state * 1664525u + 1013904223u;
}

/* Prints one NF_EXACT line: five int32 inputs followed by the five int64
 * numerators of the causal FIR (x + 2*previous + older), before division by 4. */
static void print_exact_window(uint32_t *rng)
{
    int32_t x[CORPUS_WINDOW];
    for (unsigned j = 0; j < CORPUS_WINDOW; j++)
    {
        *rng = next_lcg(*rng);
        x[j] = (int32_t)(*rng % 2000001u) - 1000000;
    }

    struct cpn_exact state = {0, 0};
    int64_t out[CORPUS_WINDOW];
    CHECK(cpn_exact(&state, x, CORPUS_WINDOW, out, CORPUS_WINDOW) == CPN_OK);

    printf("NF_EXACT");
    for (unsigned j = 0; j < CORPUS_WINDOW; j++)
    {
        printf(" %d", x[j]);
    }
    for (unsigned j = 0; j < CORPUS_WINDOW; j++)
    {
        printf(" %lld", (long long)out[j]);
    }
    puts("");
}

/* Prints NF_ROUND lines: numerator, then its quotient by 4 rounded to
 * nearest with ties to even. */
static void print_rounded_numerators(void)
{
    for (int64_t n = -CORPUS_ROUND_LIMIT; n <= CORPUS_ROUND_LIMIT; n++)
    {
        int32_t out = 0;
        CHECK(cpn_round_even(n, &out) == CPN_OK);
        printf("NF_ROUND %lld %d\n", (long long)n, out);
    }
}

static void corpus(void)
{
    uint32_t rng = UINT32_C(0x12345678);
    for (unsigned i = 0; i < CORPUS_CONVOLUTIONS; i++)
    {
        print_exact_window(&rng);
    }
    print_rounded_numerators();
}

int main(int argc, char **argv)
{
    CHECK(!fesetround(FE_TONEAREST));
    cp_test_numeric_transaction();
    cp_test_numeric();
    cp_test_numeric_policy();
    cp_test_dispatch();
    cp_test_vector();
    policies();
    guards();
    if (argc == 2 && !strcmp(argv[1], "corpus"))
    {
        corpus();
    }
    else
    {
        CHECK(argc == 1);
    }
    puts("NF_PASS");
    return 0;
}
