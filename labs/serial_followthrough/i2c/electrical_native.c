#include <stdio.h>

void ia_note(const char *s)
{
    puts(s);
}

void ia_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS %s %u %u %u %u\n", s, a, b, c, d);
}

int ia_electrical(void);
int ia_arbitration(void);

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][1])
        return 42;
    return argv[1][0] == '0'   ? ia_electrical()
           : argv[1][0] == '1' ? ia_arbitration()
                               : 42;
}
