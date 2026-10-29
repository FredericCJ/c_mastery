#include "measure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned number(const char *s)
{
    char *end;
    unsigned long v = strtoul(s, &end, 10);
    if (!*s || *end || v > 1000000)
    {
        fprintf(stderr, "CF_MEASURE invalid argument\n");
        exit(2);
    }
    return (unsigned)v;
}

int main(int argc, char **argv)
{
    if (argc == 5 && !strcmp(argv[1], "pool"))
        return cf_pool_measure(number(argv[2]), number(argv[3]), argv[4]);
    if (argc == 9 && !strcmp(argv[1], "queue"))
        return cf_queue_measure(number(argv[2]), number(argv[3]),
                                number(argv[4]), number(argv[5]),
                                number(argv[6]), number(argv[7]), argv[8]);
    if (argc == 6 && !strcmp(argv[1], "rw"))
        return cf_rw_measure(number(argv[2]), number(argv[3]), number(argv[4]),
                             argv[5]);
    return 2;
}
