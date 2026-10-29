#include <stdio.h>

void ua_note(const char *s)
{
    puts(s);
}

void ua_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    printf("OBS %s %u %u %u %u\n", s, a, b, c, d);
}

int ua_mapped(void);

int main(void)
{
    return ua_mapped();
}
