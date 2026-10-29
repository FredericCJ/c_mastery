#include "can.h"
#include <stdio.h>

void cn_note(const char *s)
{
    puts(s);
}

void cn_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS can_%s %u %u %u %u\n", s, a, b, c, d);
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '8' || argv[1][1])
    {
        return 42;
    }
    unsigned n = (unsigned)(argv[1][0] - '0');
    return n < 5 ? cn_test(n) : cn_controller_test(n);
}
