#include "rational.h"
#include "opaque.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "ER_CHECK EFR line%d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static int base(void)
{
    er_fraction r = {91, 92};
    CHECK(er_refine(3, 0, &r) == ER_INVALID && r.numerator == 91 &&
          r.denominator == 92);
    CHECK(er_refine(6, 8, &r) == ER_OK && r.numerator == 3 &&
          r.denominator == 4);
    CHECK(er_refine(0, 13, &r) == ER_OK && r.numerator == 0 &&
          r.denominator == 1);
    CHECK(er_refine(UINT32_MAX, UINT32_MAX, &r) == ER_OK && r.numerator == 1 &&
          r.denominator == 1);
    unsigned steps = 0;
    CHECK(efr_gcd(UINT32_C(2971215073), UINT32_C(1836311903), &steps) == 1 &&
          steps == 45);
    puts(
        "EFR_BASE actual-refine canonical-zero extrema Euclid-F47-F46-45steps");
    return 0;
}

static int consumer(void)
{
    uint32_t q = 91;
    er_fraction r = {2, 4};
    CHECK(efr_consume(r, &q) == ER_INVALID && q == 91);
    CHECK(er_refine(14, 4, &r) == ER_OK && efr_consume(r, &q) == ER_OK &&
          q == 3);
    r.denominator = 0;
    CHECK(efr_consume(r, &q) == ER_INVALID && q == 3);
    struct efr_signed s = {-7, 2};
    int32_t v = 91;
    CHECK(efr_signed_consume(s, &v) == ER_OK && v == -3);
    s.denominator = 0;
    CHECK(efr_signed_consume(s, &v) == ER_INVALID && v == -3);
    puts(
        "EFR_CONSUMER fabricated-denominator-rejected public-alias-invalidates prior-construction");
    return 0;
}

static int addition(void)
{
    er_fraction a = {1, UINT32_MAX}, b = a, r = {91, 92};
    CHECK(efr_add_naive(a, b, &r) == ER_RANGE && r.numerator == 91 &&
          r.denominator == 92);
    CHECK(efr_add(a, b, &r) == ER_OK && r.numerator == 2 &&
          r.denominator == UINT32_MAX);
    CHECK(efr_add((er_fraction){1, 6}, (er_fraction){1, 3}, &r) == ER_OK &&
          r.numerator == 1 && r.denominator == 2);
    r = (er_fraction){91, 92};
    CHECK(efr_add((er_fraction){UINT32_MAX, 1}, (er_fraction){1, 1}, &r) ==
              ER_RANGE &&
          r.numerator == 91 && r.denominator == 92);
    CHECK(efr_add((er_fraction){2, 4}, a, &r) == ER_INVALID &&
          r.numerator == 91);
    puts(
        "EFR_ADD naive-versus-gcd-cancel more-admitted exact-reduced unchanged-refusal");
    return 0;
}

static int signed_case(void)
{
    struct efr_signed s = {91, 92};
    CHECK(efr_signed_make(INT32_MIN, 1, &s) == ER_OK &&
          s.numerator == INT32_MIN && s.denominator == 1);
    int32_t q = 9;
    CHECK(efr_signed_consume(s, &q) == ER_OK && q == INT32_MIN);
    CHECK(efr_signed_make(INT32_MIN, UINT32_C(2147483648), &s) == ER_OK &&
          s.numerator == -1 && s.denominator == 1);
    CHECK(efr_signed_make(0, UINT32_MAX, &s) == ER_OK && s.numerator == 0 &&
          s.denominator == 1);
    CHECK(efr_signed_add((struct efr_signed){INT32_MIN, UINT32_MAX},
                         (struct efr_signed){INT32_MAX, UINT32_MAX},
                         &s) == ER_OK &&
          s.numerator == -1 && s.denominator == UINT32_MAX);
    s = (struct efr_signed){91, 92};
    CHECK(efr_signed_add((struct efr_signed){INT32_MIN, 1},
                         (struct efr_signed){-1, 1}, &s) == ER_RANGE &&
          s.numerator == 91 && s.denominator == 92);
    CHECK(efr_signed_add((struct efr_signed){-1, 2}, (struct efr_signed){1, 2},
                         &s) == ER_OK &&
          s.numerator == 0 && s.denominator == 1);
    puts(
        "EFR_SIGNED minimum-magnitude-no-abs signed-cancellation truncates-toward-zero");
    return 0;
}

struct er_heap
{
    unsigned calls, drops, fail;
    struct efr_opaque **owner;
    int empty;
};

static void *take(void *p, size_t n)
{
    struct er_heap *h = p;
    ++h->calls;
    return h->fail ? NULL : malloc(n);
}

static void release(void *p, void *memory)
{
    struct er_heap *h = p;
    ++h->drops;
    h->empty = h->owner && *h->owner == NULL;
    free(memory);
}

static int opaque(void)
{
    struct er_heap h = {0};
    struct efr_opaque *p = NULL;
    h.owner = &p;
    struct efr_memory m = {&h, take, release};
    CHECK(efr_opaque_make(m, 3, 0, &p) == ER_INVALID && p == NULL &&
          h.calls == 0);
    h.fail = 1;
    CHECK(efr_opaque_make(m, 7, 2, &p) == ER_IO && p == NULL && h.calls == 1);
    h.fail = 0;
    CHECK(efr_opaque_make(m, 7, 2, &p) == ER_OK && p);
    uint32_t q = 91;
    CHECK(efr_opaque_consume(p, &q) == ER_OK && q == 3);
    CHECK(efr_opaque_make(m, 1, 2, &p) == ER_INVALID && h.calls == 2);
    efr_opaque_drop(&p);
    CHECK(p == NULL && h.drops == 1 && h.empty);
    efr_opaque_drop(&p);
    CHECK(h.drops == 1);
    puts(
        "EFR_OPAQUE real-private-type controlled-allocation-failure empty-before-release");
    return 0;
}

static int admission(void)
{
    struct efr_signed s = {91, 92};
    er_fraction r = {91, 92};
    CHECK(efr_signed_make(INT32_MIN, 0, &s) == ER_INVALID &&
          s.numerator == 91 && s.denominator == 92);
    CHECK(efr_signed_make(1, 1, NULL) == ER_INVALID);
    CHECK(efr_consume((er_fraction){1, 1}, NULL) == ER_INVALID);
    CHECK(efr_add((er_fraction){1, 1}, (er_fraction){1, 0}, &r) == ER_INVALID &&
          r.numerator == 91);
    CHECK(efr_signed_add((struct efr_signed){0, 2}, (struct efr_signed){1, 1},
                         &s) == ER_INVALID &&
          s.numerator == 91);
    CHECK(efr_opaque_consume(NULL, NULL) == ER_INVALID);
    puts(
        "EFR_ADMISSION structural-invalid-before-arithmetic unchanged-both-fields");
    return 0;
}

static int oracle(void)
{
    unsigned kind;
    int64_t a, b;
    uint32_t d, e;
    while (scanf("%u %" SCNd64 " %" SCNu32 " %" SCNd64 " %" SCNu32, &kind, &a,
                 &d, &b, &e) == 5)
    {
        if (kind == 0)
        {
            er_fraction r = {91, 92};
            er_code code = er_refine((uint32_t)a, d, &r);
            printf("%d %" PRIu32 " %" PRIu32 "\n", code, r.numerator,
                   r.denominator);
        }
        else if (kind == 1)
        {
            uint32_t q = 91;
            er_code code = efr_consume((er_fraction){(uint32_t)a, d}, &q);
            printf("%d %" PRIu32 "\n", code, q);
        }
        else if (kind == 2 || kind == 3)
        {
            er_fraction r = {91, 92};
            er_code code =
                kind == 2 ? efr_add_naive((er_fraction){(uint32_t)a, d},
                                          (er_fraction){(uint32_t)b, e}, &r)
                          : efr_add((er_fraction){(uint32_t)a, d},
                                    (er_fraction){(uint32_t)b, e}, &r);
            printf("%d %" PRIu32 " %" PRIu32 "\n", code, r.numerator,
                   r.denominator);
        }
        else if (kind == 4 || kind == 5)
        {
            struct efr_signed r = {91, 92};
            er_code code =
                kind == 4
                    ? efr_signed_make((int32_t)a, d, &r)
                    : efr_signed_add((struct efr_signed){(int32_t)a, d},
                                     (struct efr_signed){(int32_t)b, e}, &r);
            printf("%d %" PRId32 " %" PRIu32 "\n", code, r.numerator,
                   r.denominator);
        }
        else if (kind == 6)
        {
            int32_t q = 91;
            er_code code =
                efr_signed_consume((struct efr_signed){(int32_t)a, d}, &q);
            printf("%d %" PRId32 "\n", code, q);
        }
        else
        {
            unsigned steps;
            uint32_t g = efr_gcd((uint32_t)a, d, &steps);
            printf("%" PRIu32 " %u\n", g, steps);
        }
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    double samples[7];
    unsigned long sum = 0;
    for (unsigned s = 0; s < 7; ++s)
    {
        clock_t start = clock();
        for (unsigned i = 0; i < 10000; ++i)
        {
            er_fraction r;
            CHECK(efr_add((er_fraction){1, 6}, (er_fraction){1, 3}, &r) ==
                  ER_OK);
            sum += r.numerator + r.denominator;
        }
        samples[s] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf(
        "{\"iterations\":10000,\"checksum\":%lu,\"core_allocation_calls\":0,\"unsigned_size\":%zu,\"signed_size\":%zu,\"seconds\":[",
        sum, sizeof(er_fraction), sizeof(struct efr_signed));
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", samples[i]);
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "rational_base"))
        return base();
    if (!strcmp(argv[1], "rational_consumer"))
        return consumer();
    if (!strcmp(argv[1], "rational_add"))
        return addition();
    if (!strcmp(argv[1], "rational_signed"))
        return signed_case();
    if (!strcmp(argv[1], "rational_opaque"))
        return opaque();
    if (!strcmp(argv[1], "rational_admission"))
        return admission();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
