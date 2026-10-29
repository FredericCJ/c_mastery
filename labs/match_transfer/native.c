#include "match.h"
#include <stdio.h>
#include <stdlib.h>

int mt_failure(const char *s, unsigned n)
{
    fprintf(stderr, "MT_ASSERT:%s:%u\n", s, n);
    return 41;
}

void mt_observation(const char *s, unsigned n, unsigned size)
{
    printf("MT_PASS %s cases=%u storage=%u\n", s, n, size);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    return mt_tests((unsigned)strtoul(argv[1], NULL, 10));
}
