#include "facilities.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "LP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static int add(void)
{
    int32_t v = 77;
    CHECK(lf_add(INT32_MAX, 1, &v) == LF_INVALID && v == 77);
    CHECK(lf_add(INT32_MIN, -1, &v) == LF_INVALID && v == 77);
    CHECK(lf_add(INT32_MIN, INT32_MAX, &v) == LF_OK && v == -1);
    puts("LP add validated");
    return 0;
}

static int mixed(void)
{
    uint16_t v = 77;
    CHECK(lf_mixed(-1, 65537, &v) == LF_INVALID && v == 77);
    CHECK(lf_mixed(INT32_MIN, 2147483648u, &v) == LF_OK && v == 0);
    CHECK(lf_mixed(-1, 65536, &v) == LF_OK && v == 65535);
    puts("LP mixed validated");
    return 0;
}

static int bits(void)
{
    struct lf_bits v = {9, 9, 9};
    CHECK(lf_bits(0, &v) == LF_OK && v.leading == 32 && v.trailing == 32 &&
          !v.width);
    CHECK(lf_bits(0x80000000u, &v) == LF_OK && !v.leading && v.trailing == 31 &&
          v.width == 32);
    puts("LP bits validated");
    return 0;
}

static int divide(void)
{
    uint64_t v = 77;
    CHECK(lf_div(9, 0, &v) == LF_INVALID && v == 77);
    CHECK(lf_div(UINT64_MAX, UINT64_MAX, &v) == LF_OK && v == 1);
    CHECK(lf_div(UINT64_MAX, 3, &v) == LF_OK && v == 6148914691236517205ULL);
    puts("LP divide validated");
    return 0;
}

static int select_case(void)
{
    unsigned v = 77;
    CHECK(lf_select(1, 0, &v) == LF_OK && !v);
    CHECK(lf_select(15, 0, &v) == LF_OK && v);
    CHECK(lf_select(15, 1, &v) == LF_OK && !v);
    CHECK(lf_select(31, 1, &v) == LF_OK && v);
    puts("LP select validated");
    return 0;
}

static int oracle_divide(void)
{
    uint64_t x;
    uint64_t y;
    CHECK(scanf("%" SCNu64 " %" SCNu64, &x, &y) == 2);
    uint64_t out = 77;
    enum lf_status s = lf_div(x, y, &out);
    printf("%d %" PRIu64 "\n", s, out);
    return 0;
}

static int oracle_signed_op(char op)
{
    int64_t a;
    int64_t b;
    CHECK(scanf("%" SCNd64 " %" SCNd64, &a, &b) == 2);
    if (op == 'a')
    {
        int32_t out = 77;
        enum lf_status s = lf_add((int32_t)a, (int32_t)b, &out);
        printf("%d %" PRId32 "\n", s, out);
    }
    else if (op == 'm')
    {
        uint16_t out = 77;
        enum lf_status s = lf_mixed((int32_t)a, (uint32_t)b, &out);
        printf("%d %u\n", s, out);
    }
    else if (op == 'b')
    {
        struct lf_bits out = {9, 9, 9};
        enum lf_status s = lf_bits((uint32_t)a, &out);
        printf("%d %u %u %u\n", s, out.leading, out.trailing, out.width);
    }
    else if (op == 's')
    {
        unsigned out = 77;
        enum lf_status s = lf_select((unsigned)a, (unsigned)b, &out);
        printf("%d %u\n", s, out);
    }
    else
    {
        CHECK(0);
    }
    return 0;
}

/* Read "op operand operand" records and print status plus outputs. The
   output variables start at a sentinel so an untouched output is visible. */
static int oracle(void)
{
    char op;
    while (scanf(" %c", &op) == 1)
    {
        int failed = op == 'd' ? oracle_divide() : oracle_signed_op(op);
        if (failed)
        {
            return 1;
        }
    }
    return 0;
}

static int metrics(void)
{
    puts("{\"division_steps\":64,\"bit_width\":32,\"observation_bits\":5}");
    return 0;
}

struct lp_case
{
    const char *name;
    int (*run)(void);
};

static const struct lp_case cases[] = {
    {"add", add},         {"mixed", mixed},        {"bits", bits},
    {"divide", divide},   {"select", select_case}, {"oracle", oracle},
    {"metrics", metrics},
};

/* Exit status: 0 pass, 1 failed check, 2 unknown case name. */
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            return cases[i].run();
        }
    }
    return 2;
}
