#include "channel.h"
#include <stdio.h>

void r5_note(const char *s)
{
    puts(s);
}

void r5_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS rs485_%s %u %u %u %u\n", s, a, b, c, d);
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '5' || argv[1][1])
        return 42;
    return r5_test((unsigned)(argv[1][0] - '0'));
}
