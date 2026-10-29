#include "pipeline.h"
#include <stdio.h>

void ap_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AP_ASSERT:%u:%s\n", line, s);
}

void ap_emit(const char *s, const int *v, unsigned n)
{
    printf("DATA %s", s);
    for (unsigned i = 0; i < n; ++i)
        printf(" %d", v[i]);
    putchar('\n');
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 42;
    int code = ap_test(argv[1]);
    if (!code)
        printf("OBS %s\n", argv[1]);
    return code;
}
