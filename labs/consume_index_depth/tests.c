#include "consume_index.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CI_CHECK:%s:%d\n",#x,__LINE__); return 41; } } while (0)
int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "graphs") == 0) {
        for (unsigned code = 0; code < 729; ++code)
            printf("%u %u\n", code, ci_dependency_graph(code));
        return 0;
    }
    unsigned a[4] = {11,23,37,53};
    CINode n = {0,71};
    _Atomic(CINode *) p;
    atomic_init(&p, &n);
    CHECK(ci_pointer(&p) == 71);
    for (unsigned i = 0; i < 4; ++i) {
        n.index = i;
        if (strcmp(argv[1], "value") == 0) CHECK(ci_value_index(&p,a) == a[i]);
        else if (strcmp(argv[1], "branch") == 0) CHECK(ci_branch_index(&p,a) == 23);
        else if (strcmp(argv[1], "other") == 0) {
            CHECK(ci_killed_index(&p,a) == a[i]);
            CHECK(ci_acquire_branch(&p,a) == 23);
        } else return 2;
    }
    atomic_store_explicit(&p, (CINode *)0, memory_order_release);
    CHECK(ci_pointer(&p) == 0);
    CHECK(ci_value_index(&p,a) == 0);
    CHECK(ci_branch_index(&p,a) == 11);
    CHECK(ci_killed_index(&p,a) == 0);
    CHECK(ci_acquire_branch(&p,a) == 11);
    puts("CI_PASS");
    return 0;
}
