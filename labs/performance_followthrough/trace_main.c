/*
 * Single-step trace subject for trace_gdb.py: one call of the retained
 * assembly lower bound, compared with the C reference.
 *
 *     trace_main [N [KEY]]    N in 0..8 (default 8), KEY a uint32_t (2)
 *
 * Both numbers are plain ASCII decimal (labs/support/cm_parse.h); anything
 * else, or a value out of range, exits 2 instead of tracing a different
 * request. Exit 1 means the two lower bounds disagree.
 */
#include "algo.h"
#include "cm_parse.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint32_t input[8] = {1, 2, 2, 2, 4, 6, 8, 9};

int main(int argc, char **argv)
{
    unsigned n = 8;
    unsigned key = 2;

    if ((argc > 3) ||
        ((argc > 1) && (cm_parse_unsigned(argv[1], 0, 8, &n) != CM_PARSE_OK)) ||
        ((argc > 2) &&
         (cm_parse_unsigned(argv[2], 0, UINT32_MAX, &key) != CM_PARSE_OK)))
    {
        (void)fputs("usage: trace_main [N 0..8 [KEY 0..4294967295]]\n", stderr);
        return 2;
    }
    unsigned checked = 99;
    algo_work w = {0};
    if (algo_lower(input, n, key, &checked, &w))
    {
        return 1;
    }
    unsigned r = algo_asm_lower(input, n, key);
    printf("result=%u comparisons=%llu\n", r,
           (unsigned long long)w.comparisons);
    return r != checked;
}
