#include "wrappers.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            fprintf(stderr, "LP_CHECK:%s\n", #x); \
            return 1;                             \
        }                                         \
    } while (0)

static int trace(void)
{
    struct lw_observation s;
    CHECK(lw_trace(0, &s) == 0 && s.calls == 1 && !s.text[0]);
    CHECK(lw_trace(1, &s) == 0 && s.left == 1 && !s.right &&
          !strcmp(s.text, "3"));
    CHECK(lw_trace(2, &s) == 0 && s.left == 1 && s.right == 1 && s.calls == 1 &&
          !strcmp(s.text, "3:7"));
    CHECK(lw_trace(3, &s) == 0 && !strcmp(s.text, "literal"));
    CHECK(lw_trace(4, &s) == 0 && s.left == 1 && !strcmp(s.text, "3"));
    CHECK(lw_trace(5, &s) == 0 && !s.left && !s.right && !s.calls &&
          !s.text[0]);
    return 0;
}

static int minimum(void)
{
    struct lw_observation s;
    CHECK(lw_min(0, &s) == 0 && s.value == 4);
    CHECK(lw_min(1, &s) == 0 && s.value == 3 && s.left == 2 && s.right == 1);
    CHECK(lw_min(2, &s) == 0 && s.value == 3 && s.left == 1 && s.right == 1);
    CHECK(lw_min(3, &s) == 0 && s.value == 3 && s.left == 1 && s.right == 1);
    return 0;
}

static int statement(void)
{
    struct lw_observation s;
    CHECK(lw_statement(0, &s) == 0 && s.value == 3 && s.calls == 1);
    CHECK(lw_statement(1, &s) == 0 && s.value == 7 && !s.calls);
    CHECK(lw_statement(2, &s) == 0 && s.calls == 3);
    CHECK(lw_statement(3, &s) == 0 && s.calls == 3);
    CHECK(lw_statement(4, &s) == 0 && s.value == 9);
    CHECK(lw_statement(5, &s) == 0 && s.value == 2 && s.calls == 2);
    return 0;
}

int main(int argc, char **argv)
{
    struct lw_observation s;
    unsigned limits[] = {6, 4, 6};
    int (*fn[])(unsigned, struct lw_observation *) = {lw_trace, lw_min,
                                                      lw_statement};
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "oracle"))
    {
        for (unsigned j = 0; j < 3; ++j)
            for (unsigned i = 0; i < limits[j]; ++i)
            {
                int rc = fn[j](i, &s);
                if (rc)
                    return 1;
                printf("%u %u %u %u %u %d %s\n", j, i, s.left, s.right, s.calls,
                       s.value, s.text);
            }
        return 0;
    }
    if (!strcmp(argv[1], "metrics"))
    {
        printf(
            "{\"observation_bytes\":%zu,\"text_capacity\":32,\"allocations\":0}\n",
            sizeof s);
        return 0;
    }
    for (unsigned j = 0; j < 3; ++j)
    {
        unsigned char old[sizeof s];
        memset(&s, 0x5a, sizeof s);
        memcpy(old, &s, sizeof s);
        CHECK(fn[j](limits[j], &s) == -1 && !memcmp(old, &s, sizeof s));
        CHECK(fn[j](0, NULL) == -1);
    }
    if (!strcmp(argv[1], "trace"))
    {
        CHECK(trace() == 0);
        puts("LP trace validated");
    }
    else if (!strcmp(argv[1], "minimum"))
    {
        CHECK(minimum() == 0);
        puts("LP minimum validated");
    }
    else if (!strcmp(argv[1], "statement"))
    {
        CHECK(statement() == 0);
        puts("LP statement validated");
    }
    else
        return 2;
    return 0;
}
