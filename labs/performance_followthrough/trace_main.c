#include "algo.h"
#include <stdio.h>
#include <stdlib.h>
uint32_t input[8] = {1, 2, 2, 2, 4, 6, 8, 9};

int main(int argc, char **argv)
{
    unsigned n = argc > 1 ? (unsigned)strtoul(argv[1], 0, 10) : 8,
             key = argc > 2 ? (unsigned)strtoul(argv[2], 0, 10) : 2;
    if (n > 8)
        return 1;
    unsigned checked = 99;
    algo_work w = {0};
    if (algo_lower(input, n, key, &checked, &w))
        return 1;
    unsigned r = algo_asm_lower(input, n, key);
    printf("result=%u comparisons=%llu\n", r,
           (unsigned long long)w.comparisons);
    return r != checked;
}
