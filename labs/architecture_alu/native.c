#include "alu.h"
#include <stdio.h>

void aa_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AA_ASSERT:%u:%s\n", line, s);
}

void aa_emit(const char *name, const unsigned *v, unsigned n)
{
    printf("%s", name);
    for (unsigned i = 0; i < n; ++i)
        printf(" %u", v[i]);
    puts("");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int r = aa_test(argv[1]);
    if (!r)
        printf("OBS %s\n", argv[1]);
    return r;
}
