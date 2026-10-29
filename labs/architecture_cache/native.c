#include "cache.h"
#include <stdio.h>
#include <inttypes.h>

void ach_failure(const char *s, unsigned line)
{
    fprintf(stderr, "ACH_ASSERT:%u:%s\n", line, s);
}

void ach_emit(const char *s, const uint32_t *v, size_t n)
{
    printf("DATA %s", s);
    for (size_t i = 0; i < n; ++i)
        printf(" %" PRIu32, v[i]);
    putchar('\n');
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 42;
    int code = ach_test(argv[1]);
    if (!code)
        printf("OBS %s\n", argv[1]);
    return code;
}
