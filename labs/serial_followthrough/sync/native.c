#include <stdio.h>

void su_note(const char *s)
{
    puts(s);
}

void su_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS %s %u %u %u %u\n", s, a, b, c, d);
}

int su_cases(unsigned);

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '3' || argv[1][1])
    {
        return 42;
    }
    return su_cases((unsigned)(argv[1][0] - '0'));
}
