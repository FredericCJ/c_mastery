#include "wrappers.h"
#include <stdio.h>

void aw_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AW_ASSERT:%u:%s\n", line, s);
}

void aw_observe(const char *name, const int *v, unsigned n)
{
    printf("%s", name);
    for (unsigned i = 0; i < n; ++i)
        printf(" %d", v[i]);
    puts("");
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int result = aw_test(argv[1]);
    if (!result)
        printf("OBS %s\n", argv[1]);
    return result;
}
