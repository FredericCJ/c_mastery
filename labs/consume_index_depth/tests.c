#include "consume_index.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CI_CHECK:%s:%d\n", #x, __LINE__); \
            return CHECK_FAILED;                               \
        }                                                      \
    } while (0)

enum
{
    /* 3^6: every labelled value/control/absent assignment of six edges. */
    GRAPH_CODES = 729,
    CHECK_FAILED = 41,
    USAGE_ERROR = 2,
    ARRAY_ELEMENTS = 4
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return USAGE_ERROR;
    }
    if (strcmp(argv[1], "graphs") == 0)
    {
        for (unsigned code = 0; code < GRAPH_CODES; ++code)
        {
            printf("%u %u\n", code, ci_dependency_graph(code));
        }
        return 0;
    }
    /* Distinct values so a wrong index is visible; 71 is the node's payload. */
    unsigned a[ARRAY_ELEMENTS] = {11, 23, 37, 53};
    struct ci_node n = {0, 71};
    _Atomic(struct ci_node *) p;
    atomic_init(&p, &n);
    CHECK(ci_pointer(&p) == 71);
    for (unsigned i = 0; i < ARRAY_ELEMENTS; ++i)
    {
        n.index = i;
        if (strcmp(argv[1], "value") == 0)
        {
            CHECK(ci_value_index(&p, a) == a[i]);
        }
        else if (strcmp(argv[1], "branch") == 0)
        {
            CHECK(ci_branch_index(&p, a) == 23);
        }
        else if (strcmp(argv[1], "other") == 0)
        {
            CHECK(ci_killed_index(&p, a) == a[i]);
            CHECK(ci_acquire_branch(&p, a) == 23);
        }
        else
        {
            return USAGE_ERROR;
        }
    }
    atomic_store_explicit(&p, (struct ci_node *)0, memory_order_release);
    CHECK(ci_pointer(&p) == 0);
    CHECK(ci_value_index(&p, a) == 0);
    CHECK(ci_branch_index(&p, a) == 11);
    CHECK(ci_killed_index(&p, a) == 0);
    CHECK(ci_acquire_branch(&p, a) == 11);
    puts("CI_PASS");
    return 0;
}
