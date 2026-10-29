#include "shunting.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "SY_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static struct pd_tokens literal(unsigned value)
{
    struct pd_tokens t = {0};
    t.count = 2;
    t.tokens[0] = (struct pd_token){0, (int32_t)value, 0, 1, 1, 1};
    t.tokens[1] = (struct pd_token){7, 0, 1, 0, 1, 2};
    return t;
}

static int semantics(void)
{
    static const char *text[] = {
        "2^3^2", "-2^2",   "(-2)^2", "9-4-2",        "2*3+4",       "--3",
        "2^--3", "2^-3^2", "0^0",    "2147483647+1", "(1+2)*(3-4)", "2^3^2"};
    static const int expected[] = {512, -4, 4, 3, 10, 3, 8, 0, 1, 0, -3, 64};
    static const int status[] = {0, 0,         0, 0,        0, 0,
                                 0, PD_DOMAIN, 0, PD_RANGE, 0, 0};
    struct pd_tokens t = literal(7);
    struct pd_tree tree, pratt;
    struct sy_cost cost;
    struct pd_diagnostic d;
    CHECK(sy_parse(&t, 1, 64, 64, &tree, &cost, &d) == PD_OK);
    CHECK(tree.count == 1 && tree.nodes[0].value == 7);
    for (unsigned i = 0; i < sizeof(text) / sizeof(text[0]); ++i)
    {
        unsigned right = i == 11 ? 0 : 1;
        int32_t value = 123;
        CHECK(pd_lex((const unsigned char *)text[i], strlen(text[i]), &t, &d) ==
              PD_OK);
        CHECK(sy_parse(&t, right, 64, 64, &tree, &cost, &d) == PD_OK);
        CHECK(pd_pratt(&t, right, &pratt, &d) == PD_OK);
        CHECK(tree.count == pratt.count && tree.root == pratt.root);
        for (unsigned n = 0; n < tree.count; ++n)
        {
            struct pd_expr a = tree.nodes[n], b = pratt.nodes[n];
            CHECK(a.kind == b.kind && a.left == b.left && a.right == b.right &&
                  a.value == b.value && a.offset == b.offset);
        }
        CHECK(pd_evaluate(&tree, &value) == status[i]);
        CHECK(value == (status[i] ? 123 : expected[i]));
        CHECK(cost.reductions <= cost.pushes && cost.pushes < t.count);
    }
    return 0;
}

static int bounds(void)
{
    static const char *bad[] = {"",   "()", "1 2",  "2(3)", "(1",    "1)",
                                "1+", "*2", "2^^3", "(-)",  "(1)(2)"};
    struct pd_tokens t = literal(1);
    struct pd_tree tree, saved;
    struct sy_cost cost, oldcost;
    struct pd_diagnostic d;
    CHECK(sy_parse(&t, 0, 0, 1, &tree, &cost, &d) == PD_OK);
    memset(&tree, 0xa5, sizeof tree);
    saved = tree;
    memset(&cost, 0x5a, sizeof cost);
    oldcost = cost;
    CHECK(sy_parse(&t, 1, 64, 0, &tree, &cost, &d) == PD_RANGE);
    CHECK(memcmp(&tree, &saved, sizeof tree) == 0 &&
          memcmp(&cost, &oldcost, sizeof cost) == 0);
    for (unsigned i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i)
    {
        CHECK(pd_lex((const unsigned char *)bad[i], strlen(bad[i]), &t, &d) ==
              PD_OK);
        CHECK(sy_parse(&t, 1, 64, 64, &tree, &cost, &d) == PD_INVALID);
        CHECK(memcmp(&tree, &saved, sizeof tree) == 0 &&
              memcmp(&cost, &oldcost, sizeof cost) == 0);
    }
    CHECK(pd_lex((const unsigned char *)"1+2*3", 5, &t, &d) == PD_OK);
    CHECK(sy_parse(&t, 1, 1, 64, &tree, &cost, &d) == PD_RANGE);
    CHECK(memcmp(&tree, &saved, sizeof tree) == 0);
    CHECK(sy_parse(&t, 1, 64, 4, &tree, &cost, &d) == PD_RANGE);
    CHECK(memcmp(&tree, &saved, sizeof tree) == 0);
    CHECK(sy_parse(&t, 1, 64, 5, &tree, &cost, &d) == PD_OK && tree.count == 5);
    CHECK(sy_parse(&t, 2, 64, 64, &tree, &cost, &d) == PD_INVALID);
    CHECK(sy_parse(&t, 1, 65, 64, &tree, &cost, &d) == PD_INVALID);
    CHECK(sy_parse(NULL, 1, 64, 64, &tree, &cost, &d) == PD_INVALID);
    t = literal(1);
    t.tokens[0].value = -1;
    CHECK(sy_parse(&t, 1, 64, 64, &tree, &cost, &d) == PD_INVALID);
    t = literal(1);
    t.count = 1;
    CHECK(sy_parse(&t, 1, 64, 64, &tree, &cost, &d) == PD_INVALID);
    t = literal(1);
    t.tokens[0].kind = 7;
    CHECK(sy_parse(&t, 1, 64, 64, &tree, &cost, &d) == PD_INVALID);
    {
        unsigned char nested[64];
        for (unsigned i = 0; i < 31; ++i)
        {
            nested[i] = '(';
        }
        nested[31] = '1';
        for (unsigned i = 32; i < 63; ++i)
        {
            nested[i] = ')';
        }
        CHECK(pd_lex(nested, 63, &t, &d) == PD_OK);
        CHECK(sy_parse(&t, 1, 31, 1, &tree, &cost, &d) == PD_OK &&
              cost.peak_operators == 31);
        CHECK(sy_parse(&t, 1, 30, 1, &tree, &cost, &d) == PD_RANGE);
    }
    return 0;
}

static int stream(void)
{
    struct sy_state s, saved;
    struct pd_tree tree, whole, old;
    struct sy_cost c, wc;
    struct pd_diagnostic d;
    struct pd_tokens t;
    CHECK(sy_begin(1, 64, 64, &s) == PD_OK);
    memset(&tree, 0xa5, sizeof tree);
    old = tree;
    CHECK(sy_result(&s, &tree, &c) == PD_PENDING &&
          !memcmp(&tree, &old, sizeof tree));
    CHECK(pd_lex((const unsigned char *)"1+2*3", 5, &t, &d) == PD_OK);
    CHECK(sy_parse(&t, 1, 64, 64, &whole, &wc, &d) == PD_OK);
    for (size_t i = 0; i < t.count; ++i)
    {
        CHECK(sy_push(&s, &t.tokens[i], &d) ==
              (i + 1 == t.count ? PD_OK : PD_PENDING));
    }
    CHECK(sy_result(&s, &tree, &c) == PD_OK && tree.count == whole.count &&
          tree.root == whole.root);
    CHECK(c.pushes == wc.pushes && c.reductions == wc.reductions);
    saved = s;
    CHECK(sy_push(&s, &t.tokens[0], &d) == PD_INVALID &&
          !memcmp(&s, &saved, sizeof s));
    /* A rejected EOF can reduce operators privately before discovering an
       unmatched bracket. The retained prefix must still be resumable. */
    CHECK(pd_lex((const unsigned char *)"(1+2", 4, &t, &d) == PD_OK);
    CHECK(sy_begin(1, 64, 64, &s) == PD_OK);
    for (size_t i = 0; i + 1 < t.count; ++i)
    {
        CHECK(sy_push(&s, &t.tokens[i], &d) == PD_PENDING);
    }
    saved = s;
    CHECK(sy_push(&s, &t.tokens[t.count - 1], &d) == PD_INVALID);
    CHECK(!memcmp(&s, &saved, sizeof s));
    {
        struct pd_token close = {4, 0, 4, 1, 1, 5};
        CHECK(sy_push(&s, &close, &d) == PD_PENDING);
    }
    CHECK(sy_push(&s, &t.tokens[t.count - 1], &d) == PD_OK);
    CHECK(sy_result(&s, &tree, &c) == PD_OK);
    {
        int32_t value;
        CHECK(pd_evaluate(&tree, &value) == PD_OK && value == 3);
    }
    CHECK(pd_lex((const unsigned char *)"1+2*3", 5, &t, &d) == PD_OK);
    CHECK(sy_begin(1, 1, 5, &s) == PD_OK);
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(sy_push(&s, &t.tokens[i], &d) == PD_PENDING);
    }
    saved = s;
    CHECK(sy_push(&s, &t.tokens[3], &d) == PD_RANGE &&
          !memcmp(&s, &saved, sizeof s));
    t.tokens[3].kind = 1;
    CHECK(sy_push(&s, &t.tokens[3], NULL) == PD_PENDING);
    CHECK(sy_push(&s, &t.tokens[4], NULL) == PD_PENDING);
    CHECK(sy_push(&s, &t.tokens[5], NULL) == PD_OK);
    CHECK(sy_result(&s, &tree, &c) == PD_OK);
    {
        int32_t value;
        CHECK(pd_evaluate(&tree, &value) == PD_OK && value == 6);
    }
    /* Token-count boundary is independent of AST/operator capacities. */
    CHECK(sy_begin(1, 64, 64, &s) == PD_OK);
    {
        struct pd_token minus = {5, 0, 0, 1, 1, 1};
        for (unsigned i = 0; i < 64; ++i)
        {
            CHECK(sy_push(&s, &minus, &d) == PD_PENDING);
        }
        saved = s;
        CHECK(sy_push(&s, &minus, &d) == PD_RANGE &&
              !memcmp(&s, &saved, sizeof s));
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "semantics"))
    {
        return semantics();
    }
    if (!strcmp(argv[1], "bounds"))
    {
        return bounds();
    }
    if (!strcmp(argv[1], "stream"))
    {
        return stream();
    }
    return 2;
}
