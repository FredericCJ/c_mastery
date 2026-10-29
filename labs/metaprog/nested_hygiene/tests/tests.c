#include "nested_hygiene.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "MPO_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static struct mph_instruction ins(unsigned op, unsigned target, unsigned mask,
                                  unsigned symbol, unsigned context)
{
    return (struct mph_instruction){op, target, mask, {0, symbol, context}};
}

static struct mph_program repeated(void)
{
    struct mph_program p = {0};
    p.count = 2;
    p.definitions[0].count = 1;
    p.definitions[0].code[0] = ins(MPH_REPEAT, 1, 3, 0, 0);
    p.definitions[1].count = 3;
    p.definitions[1].code[0] = ins(MPH_INTRODUCED, 0, 0, 7, 0);
    p.definitions[1].code[1] = ins(MPH_ARGUMENT, 0, 0, 0, 0);
    p.definitions[1].code[2] = ins(MPH_ARGUMENT, 1, 0, 0, 0);
    return p;
}

static void arguments(struct mph_arguments a[2])
{
    memset(a, 0, 2 * sizeof *a);
    a[0].count = a[1].count = 2;
    a[0].tokens[0] = (struct mp_token){0, 7, 5};
    a[0].tokens[1] = (struct mp_token){0, 8, 5};
    a[1].tokens[0] = (struct mp_token){0, 7, 6};
    a[1].tokens[1] = (struct mp_token){0, 9, 6};
}

static int nested(void)
{
    struct mph_program p = repeated();
    struct mph_arguments a[2];
    arguments(a);
    struct mph_expansion out = {0}, old;
    unsigned next = 12;
    struct mph_budget b = {8, 64, 256, 0};
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK && out.count == 6 &&
          next == 15 && out.invocations == 3 && out.steps == 10 &&
          out.peak_depth == 2);
    CHECK(out.tokens[0].context == 13 && out.tokens[1].context == 5 &&
          out.tokens[2].context == 6 && out.tokens[3].context == 14 &&
          out.tokens[4].symbol == 8);
    old = out;
    next = 12;
    a[1].count = 1;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_SHAPE && next == 12 &&
          !memcmp(&old, &out, sizeof out));
    a[0].count = a[1].count = 0;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK && out.count == 0 &&
          next == 13 && out.steps == 2);
    arguments(a);
    p.count = 3;
    p.definitions[0].code[0] = ins(MPH_REPEAT, 1, 1, 0, 0);
    p.definitions[1].count = 1;
    p.definitions[1].code[0] = ins(MPH_REPEAT, 2, 2, 0, 0);
    p.definitions[2].count = 2;
    p.definitions[2].code[0] = ins(MPH_ARGUMENT, 0, 0, 0, 0);
    p.definitions[2].code[1] = ins(MPH_ARGUMENT, 1, 0, 0, 0);
    next = 12;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK && out.count == 8 &&
          next == 19 && out.peak_depth == 3 && out.tokens[0].symbol == 7 &&
          out.tokens[1].symbol == 7 && out.tokens[2].symbol == 7 &&
          out.tokens[3].symbol == 9 && out.tokens[4].symbol == 8);
    puts(
        "MPH_NESTED fresh-invocations zipped-repetition nested-binding-indices");
    return 0;
}

static int budgets(void)
{
    struct mph_program p = {0};
    struct mph_arguments a[2] = {0};
    struct mph_expansion out = {0}, old;
    unsigned next = 12;
    struct mph_budget b = {1, 64, 256, 0};
    p.count = 2;
    p.definitions[0].count = 1;
    p.definitions[0].code[0] = ins(MPH_CALL, 1, 0, 0, 0);
    old = out;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_BUDGET && next == 12 &&
          !memcmp(&old, &out, sizeof out));
    b.depth = 2;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK && next == 14 &&
          out.steps == 3);
    p = repeated();
    arguments(a);
    next = 12;
    b = (struct mph_budget){8, 5, 256, 0};
    old = out;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_BUDGET && next == 12 &&
          !memcmp(&old, &out, sizeof out));
    b.tokens = 6;
    b.steps = 9;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_BUDGET && next == 12);
    b.steps = 10;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK && next == 15);
    next = UINT_MAX - 1;
    old = out;
    b = (struct mph_budget){8, 64, 256, 0};
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_EXHAUSTED &&
          next == UINT_MAX - 1 && !memcmp(&old, &out, sizeof out));
    p = (struct mph_program){0};
    p.count = 1;
    p.definitions[0].count = 1;
    p.definitions[0].code[0] = ins(MPH_CALL, 0, 0, 0, 0);
    next = 12;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_BUDGET && next == 12);
    p.definitions[0].count = 0;
    next = UINT_MAX - 1;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK &&
          next == UINT_MAX);
    old = out;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_EXHAUSTED &&
          next == UINT_MAX && !memcmp(&old, &out, sizeof out));
    puts(
        "MPH_BUDGET depth-token-step limits context-exhaustion unchanged-publication");
    return 0;
}

static int capture(void)
{
    struct mph_program p = {0};
    struct mph_arguments a[2] = {0};
    struct mph_expansion out = {0};
    unsigned next = 12;
    int value = 99;
    struct mph_budget b = {8, 64, 256, 0};
    struct mph_binding env[] = {{{0, 7, 5}, 11}, {{0, 7, 12}, 99}};
    p.count = 1;
    p.definitions[0].count = 2;
    p.definitions[0].code[0] = ins(MPH_INTRODUCED, 0, 0, 7, 0);
    p.definitions[0].code[1] = ins(MPH_ESCAPE, 0, 0, 7, 0);
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_CAPTURE && next == 12);
    b.allow_capture = 1;
    CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK &&
          out.tokens[0].context == 12 && out.tokens[1].context == 5);
    CHECK(mph_resolve(env, 2, out.tokens[0], &value) == MPH_OK && value == 99);
    CHECK(mph_resolve(env, 2, out.tokens[1], &value) == MPH_OK && value == 11);
    struct mp_token flawed = out.tokens[1];
    flawed.context = 12;
    CHECK(mph_resolve(env, 2, flawed, &value) == MPH_OK && value == 99);
    value = 77;
    CHECK(mph_resolve(env, 2, (struct mp_token){0, 8, 5}, &value) ==
              MPH_NOT_FOUND &&
          value == 77);
    puts(
        "MPH_CAPTURE exact-environment-identity explicit-parent-context-escape");
    return 0;
}

static int base(void)
{
    struct mp_token pattern[] = {{2, 7, 0}, {1, 0, 0}, {2, 7, 0}},
                    arg = {0, 7, 5}, out[3] = {{0, 0, 0}};
    unsigned next = 12;
    size_t used = 99;
    CHECK(mp_expand(pattern, 3, &arg, 1, &next, out, 3, &used) && next == 13 &&
          used == 3 && out[0].context == 12 && out[1].context == 5 &&
          out[2].context == 12);
    next = UINT_MAX;
    used = 99;
    CHECK(!mp_expand(pattern, 3, &arg, 1, &next, out, 3, &used) &&
          next == UINT_MAX && used == 99);
    puts("MPH_BASE actual-token-transform caller-context finite-freshness");
    return 0;
}

static int oracle(void)
{
    unsigned entry, caller, next;
    size_t defs;
    while (scanf("%zu %u %u %u", &defs, &entry, &next, &caller) == 4)
    {
        struct mph_program p = {0};
        struct mph_arguments a[2] = {0};
        struct mph_expansion out = {0};
        struct mph_budget b;
        out.count = 777;
        p.count = defs;
        if (defs > 4 || scanf("%u %u %u %u", &b.depth, &b.tokens, &b.steps,
                              &b.allow_capture) != 4)
            return 2;
        for (size_t i = 0; i < 2; ++i)
        {
            if (scanf("%zu", &a[i].count) != 1 || a[i].count > 4)
                return 2;
            for (size_t j = 0; j < a[i].count; ++j)
                if (scanf("%u %u %u", &a[i].tokens[j].kind,
                          &a[i].tokens[j].symbol, &a[i].tokens[j].context) != 3)
                    return 2;
        }
        for (size_t i = 0; i < defs; ++i)
        {
            if (scanf("%zu", &p.definitions[i].count) != 1 ||
                p.definitions[i].count > 16)
                return 2;
            for (size_t j = 0; j < p.definitions[i].count; ++j)
            {
                struct mph_instruction *n = &p.definitions[i].code[j];
                if (scanf("%u %u %u %u %u %u", &n->operation, &n->target,
                          &n->mask, &n->token.kind, &n->token.symbol,
                          &n->token.context) != 6)
                    return 2;
            }
        }
        int status = mph_expand(&p, entry, a, caller, b, &next, &out);
        printf("%d %u %zu %u %u %u", status, next, out.count, out.invocations,
               out.steps, out.peak_depth);
        if (status == MPH_OK)
            for (size_t i = 0; i < out.count; ++i)
                printf(" %u %u %u", out.tokens[i].kind, out.tokens[i].symbol,
                       out.tokens[i].context);
        putchar('\n');
    }
    return 0;
}

static int metrics(void)
{
    struct mph_program p = repeated();
    struct mph_arguments a[2];
    arguments(a);
    struct mph_expansion out;
    struct mph_budget b = {8, 64, 256, 0};
    double seconds[7];
    uint64_t sum = 0;
    for (unsigned sample = 0; sample < 7; ++sample)
    {
        clock_t start = clock();
        for (unsigned i = 0; i < 10000; ++i)
        {
            unsigned next = 12;
            CHECK(mph_expand(&p, 0, a, 5, b, &next, &out) == MPH_OK);
            sum += out.count;
        }
        seconds[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf("{\"iterations\":10000,\"checksum\":%" PRIu64
           ",\"program_size\":%zu,\"result_size\":%zu,\"seconds\":[",
           sum, sizeof p, sizeof out);
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", seconds[i]);
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "macro_nested"))
        return nested();
    if (!strcmp(argv[1], "macro_budgets"))
        return budgets();
    if (!strcmp(argv[1], "macro_capture"))
        return capture();
    if (!strcmp(argv[1], "macro_base"))
        return base();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
