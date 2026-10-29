#include "transceivers.h"
#include <stdio.h>
#include <stdlib.h>

void tr_note(const char *s)
{
    puts(s);
}

void tr_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS transceivers_%s %u %u %u %u\n", s, a, b, c, d);
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '5' || argv[1][1])
    {
        return 42;
    }
    return tr_test((unsigned)(argv[1][0] - '0'));
}
