#include "migration.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "LP_CHECK %d %s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void sum(void)
{
    int out = 77;
    CHECK(lp_sum(INT_MAX, 1, &out) == LP_RANGE && out == 77);
    CHECK(lp_sum(INT_MIN, -1, &out) == LP_RANGE && out == 77);
    CHECK(lp_sum(INT_MIN, INT_MAX, &out) == LP_OK && out == -1);
    CHECK(lp_sum(1, 2, NULL) == LP_INVALID);
    CHECK(lp_sum(0, 0, &out) == LP_OK && out == 0);
}

static void adapter(void)
{
    struct lp_record a = {INT_MIN, 0};
    struct lp_record b = {INT_MAX, 1};
    struct lp_record z = {0, 2};
    CHECK(lp_compare(&a, &b) == -1 && lp_adapter(&a, &b) == -1);
    CHECK(lp_adapter(&b, &a) == 1 && lp_adapter(&z, &z) == 0);
    struct lp_record values[3] = {b, z, a};
    qsort(values, 3, sizeof(values[0]), lp_adapter);
    CHECK(values[0].key == INT_MIN && values[1].key == 0 &&
          values[2].key == INT_MAX);
}

static void reverse(void)
{
    struct lp_order order = {1};
    struct lp_record a = {INT_MIN, 0};
    struct lp_record b = {INT_MAX, 1};
    CHECK(lp_context_adapter(&a, &b, &order) == 1);
    struct lp_record values[3] = {a, {0, 2}, b};
    CHECK(lp_sort(values, 3, &order) == LP_OK && values[0].key == INT_MAX &&
          values[2].key == INT_MIN);
    CHECK(lp_sort(NULL, 0, &order) == LP_OK);
    order.reverse = 2;
    CHECK(lp_sort(values, 3, &order) == LP_INVALID && values[0].key == INT_MAX);
}

static void stable(void)
{
    struct lp_order order = {0};
    struct lp_record values[4] = {{7, 0}, {0, 1}, {7, 2}, {0, 3}};
    CHECK(lp_sort(values, 4, &order) == LP_OK && values[0].serial == 1 &&
          values[1].serial == 3 && values[2].serial == 0 &&
          values[3].serial == 2);
    CHECK(lp_sort(values, 17, &order) == LP_RANGE);
}

enum
{
    ORACLE_MAX_RECORDS = 16,
    NOT_SET = 77
};

/* Read "reverse n key..." records, sort with lp_sort and print each key with
   its original position, so stability is visible. */
static void sort_oracle(void)
{
    unsigned reverse_order;
    size_t n;
    while (scanf("%u %zu", &reverse_order, &n) == 2)
    {
        CHECK(n <= ORACLE_MAX_RECORDS);
        struct lp_record values[ORACLE_MAX_RECORDS];
        for (size_t i = 0; i < n; ++i)
        {
            CHECK(scanf("%d", &values[i].key) == 1);
            values[i].serial = (unsigned)i;
        }
        struct lp_order order = {reverse_order};
        CHECK(lp_sort(values, n, &order) == LP_OK);
        for (size_t i = 0; i < n; ++i)
        {
            printf("%d:%u ", values[i].key, values[i].serial);
        }
        puts("");
    }
}

static void sum_oracle(void)
{
    int a;
    int b;
    while (scanf("%d %d", &a, &b) == 2)
    {
        int out = NOT_SET;
        int status = lp_sum(a, b, &out);
        printf("%d %d\n", status, out);
    }
}

static void metrics(void)
{
    printf("{\"record_bytes\":%zu,\"int_bits\":%zu,\"capacity\":16}\n",
           sizeof(struct lp_record), sizeof(int) * CHAR_BIT);
}

struct lp_case
{
    const char *name;
    void (*run)(void);
    int prints_verdict; /* 1: print "LP <name> validated" after success */
};

static const struct lp_case cases[] = {
    {"oracle", sort_oracle, 0}, {"sum_oracle", sum_oracle, 0},
    {"metrics", metrics, 0},    {"sum", sum, 1},
    {"adapter", adapter, 1},    {"reverse", reverse, 1},
    {"stable", stable, 1}};

/* Exit status: 0 pass, 1 failed check (CHECK exits). */
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            cases[i].run();
            if (cases[i].prints_verdict)
            {
                printf("LP %s validated\n", argv[1]);
            }
            return 0;
        }
    }
    CHECK(0);
    return 1;
}
