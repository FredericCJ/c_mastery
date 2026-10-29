#include <stdio.h>

void ua_note(const char *s)
{
    puts(s);
}

void ua_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS %s %u %u %u %u\n", s, a, b, c, d);
}

int ua_cases(unsigned);

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '4' || argv[1][1])
        return 42;
    return ua_cases((unsigned)(argv[1][0] - '0'));
}
