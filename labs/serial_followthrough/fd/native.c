#include "fd.h"
#include <stdio.h>

void fd_note(const char *s)
{
    puts(s);
}

void fd_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS fd_%s %u %u %u %u\n", s, a, b, c, d);
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '3' || argv[1][1])
    {
        return 42;
    }
    return fd_test((unsigned)(argv[1][0] - '0'));
}
