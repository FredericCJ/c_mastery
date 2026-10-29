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
static unsigned guarded;

static void guards(void)
{
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0 && (size_t)page >= (CPN_BLOCK + 4) * sizeof(double));
    size_t bytes = (size_t)page;
    unsigned char *im = mmap(0, 3 * bytes, PROT_NONE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
                  *om = mmap(0, 3 * bytes, PROT_NONE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(im != MAP_FAILED && om != MAP_FAILED);
    CHECK(!mprotect(im + bytes, bytes, PROT_READ | PROT_WRITE) &&
          !mprotect(om + bytes, bytes, PROT_READ | PROT_WRITE));
    for (unsigned backend = 0; backend < 4; backend++)
    {
        struct cpv_plan plan;
        if (cpv_plan(&plan, (enum cpv_backend)backend) == CPN_UNSUPPORTED)
            continue;
        for (size_t n = 0; n <= 9; n++)
            for (unsigned side = 0; side < 2; side++)
                for (size_t offset = 0; offset < 4; offset++)
                {
                    double *in =
                        (double *)(void *)(im + (side ? bytes : 2 * bytes)) +
                        (side ? (ptrdiff_t)offset : -(ptrdiff_t)(n + offset));
                    double *out = (double *)(void *)(om + 2 * bytes) - n;
                    for (size_t i = 0; i < n; i++)
                        in[i] = (double)((int)i - 3) / 8;
                    double expected[9];
                    struct cpn_double reference = {4, -8}, actual = reference;
                    CHECK(cpn_double(&reference, in, n, expected, n) == CPN_OK);
                    CHECK(cpv_run(&plan, &actual, in, n, out, n) == CPN_OK);
                    CHECK(!memcmp(expected, out, n * sizeof(double)) &&
                          actual.previous == reference.previous &&
                          actual.older == reference.older);
                    guarded++;
                }
    }
    CHECK(!munmap(im, 3 * bytes) && !munmap(om, 3 * bytes));
    printf("NF_GUARDS %u\n", guarded);
}

static void policies(void)
{
    struct nf_error e = {7, 8, 9};
    CHECK(!nf_compare(NF_TELEMETRY, 0, 0x1p-44, &e) && e.accepted &&
          e.absolute == 0x1p-44);
    CHECK(!nf_compare(NF_AUDIT, 0, 0x1p-44, &e) && !e.accepted);
    CHECK(!nf_compare(NF_TELEMETRY, 0, -0.0, &e) && e.accepted);
    CHECK(!nf_compare(NF_AUDIT, 0, -0.0, &e) && !e.accepted);
    CHECK(!nf_compare(NF_TELEMETRY, 1, 0, &e) && !e.accepted &&
          e.absolute == 1);
    struct nf_error old = e;
    CHECK(nf_compare(NF_TELEMETRY, NAN, 0, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
    CHECK(nf_compare(NF_TELEMETRY, 0, INFINITY, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
    CHECK(nf_compare((enum nf_policy)99, 0, 0, &e) == -1 &&
          !memcmp(&e, &old, sizeof e));
    double input[] = {-0x1p55, 0x1p54, 4};
    struct cpn_double s = {0, 0};
    double out[3];
    CHECK(cpn_double(&s, input, 3, out, 3) == CPN_OK);
    volatile double reordered =
        (.5 * input[1] + .25 * input[0]) + .25 * input[2];
    CHECK(out[2] == 0 && reordered == 1);
    CHECK(!nf_compare(NF_AUDIT, out[2], reordered, &e) && !e.accepted);
    printf("NF_CANCELLATION scalar=%a reordered=%a rational=1 absolute=%a\n",
           out[2], (double)reordered, e.absolute);
    /* Smallest binary64 subnormal is preserved by identity additions in this profile. */
    volatile double tiny = 0x1p-1074, zero = 0;
    CHECK(tiny + zero == tiny);
    CHECK(fegetround() == FE_TONEAREST);
    puts("NF_POLICY PASS telemetry-absolute-plus-relative audit-bits");
}

static void corpus(void)
{
    uint32_t rng = UINT32_C(0x12345678);
    for (unsigned i = 0; i < 4096; i++)
    {
        int32_t x[5];
        for (unsigned j = 0; j < 5; j++)
        {
            rng = rng * 1664525u + 1013904223u;
            x[j] = (int32_t)(rng % 2000001u) - 1000000;
        }
        struct cpn_exact s = {0, 0};
        int64_t out[5];
        CHECK(cpn_exact(&s, x, 5, out, 5) == CPN_OK);
        printf("NF_EXACT");
        for (unsigned j = 0; j < 5; j++)
            printf(" %d", x[j]);
        for (unsigned j = 0; j < 5; j++)
            printf(" %lld", (long long)out[j]);
        puts("");
    }
    for (int64_t n = -10001; n <= 10001; n++)
    {
        int32_t out = 0;
        CHECK(cpn_round_even(n, &out) == CPN_OK);
        printf("NF_ROUND %lld %d\n", (long long)n, out);
    }
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
        corpus();
    else
        CHECK(argc == 1);
    puts("NF_PASS");
    return 0;
}
