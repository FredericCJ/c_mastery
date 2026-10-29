#include <stdlib.h>
#include <stdio.h>
/* A static-link wrapper catches undefined malloc references emitted by linked
 * members; source inspection is still needed for direct/internal storage. */
void *__real_malloc(size_t);
static unsigned tracing, requests;

void *__wrap_malloc(size_t n)
{
    if (tracing)
        ++requests;
    return __real_malloc(n);
}

static int cmp(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

int main(void)
{
    int a[300];
    for (unsigned i = 0; i < 300; ++i)
        a[i] = (int)(300 - i);
    tracing = 1;
    qsort(a, 300, sizeof *a, cmp);
    tracing = 0;
    for (unsigned i = 1; i < 300; ++i)
        if (a[i - 1] > a[i])
            return 1;
    if (requests)
        return 2;
    printf("{\"malloc_hook_requests\":%u,\"sorted\":true}\n", requests);
    return 0;
}
