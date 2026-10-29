#include "commit.h"
#include <stdio.h>

void at_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AT_ASSERT:%u:%s\n", line, s);
}

void at_emit(const char *s, const unsigned *v, unsigned n)
{
    printf("DATA %s", s);
    for (unsigned i = 0; i < n; ++i)
    {
        printf(" %u", v[i]);
    }
    putchar('\n');
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 42;
    }
    int code = at_test(argv[1]);
    if (!code)
    {
        printf("OBS %s\n", argv[1]);
    }
    return code;
}
