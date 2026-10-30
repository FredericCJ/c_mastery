/*
 * Observation of malloc requests made while qsort sorts 300 integers, linked
 * statically against musl with `-Wl,--wrap=malloc`.
 *
 * Record how many requests the wrapper saw and which objects in the link
 * could have made them; compare that with the storage the library source
 * shows qsort using. Exit status: 1 when the array is not sorted, 2 when the
 * wrapper saw a request.
 */
#include <stdlib.h>
#include <stdio.h>

#define COUNT 300u

void *__real_malloc(size_t bytes);

static unsigned tracing;
static unsigned requests;

void *__wrap_malloc(size_t bytes)
{
    if (tracing)
    {
        ++requests;
    }
    return __real_malloc(bytes);
}

static int compare_int(const void *left, const void *right)
{
    int x = *(const int *)left;
    int y = *(const int *)right;
    return (x > y) - (x < y);
}

int main(void)
{
    int values[COUNT];
    for (unsigned i = 0; i < COUNT; ++i)
    {
        values[i] = (int)(COUNT - i);
    }
    tracing = 1;
    qsort(values, COUNT, sizeof *values, compare_int);
    tracing = 0;
    for (unsigned i = 1; i < COUNT; ++i)
    {
        if (values[i - 1] > values[i])
        {
            return 1;
        }
    }
    if (requests)
    {
        return 2;
    }
    printf("{\"malloc_hook_requests\":%u,\"sorted\":true}\n", requests);
    return 0;
}
