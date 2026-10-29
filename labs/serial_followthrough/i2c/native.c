#include <stdio.h>

void ia_note(const char *s)
{
    puts(s);
}

void ia_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS %s %u %u %u %u\n", s, a, b, c, d);
}

int ia_cases(unsigned);

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][1] || argv[1][0] < '0' || argv[1][0] > '5')
    {
        return 42;
    }
    return ia_cases((unsigned)(argv[1][0] - '0'));
}
