/* Defined-state defect: transient count exceeds capacity, then a symptom patch
 * clamps it. */
#include <stddef.h>
#include <stdio.h>
#ifndef REPAIRED
#define REPAIRED 0
#endif
volatile unsigned violations;

__attribute__((noinline)) int admit(size_t *count, size_t capacity)
{
    if (REPAIRED)
    {
        if (*count >= capacity)
            return 0;
        (*count)++;
        return 1;
    }
    (*count)++;
    if (*count > capacity)
    {
        violations++;
        *count = capacity;
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    size_t count = 4;
    int result = 0;
    const char *events = argc == 2 ? argv[1] : "A";
    size_t length = 0;
    while (events[length])
    {
        if (++length > 64)
            return 2;
    }
    for (size_t i = 0; i < length; i++)
    {
        if (events[i] != 'A' && events[i] != 'N')
            return 2;
    }
    for (size_t i = 0; i < length; i++)
        if (events[i] == 'A')
            result = admit(&count, 4);
    printf("count=%zu result=%d violations=%u\n", count, result, violations);
    return violations ? 1 : 0;
}
