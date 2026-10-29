#include "buffer.h"
#include <stdio.h>

int main(void)
{
    int storage[16], sum = 77;
    unsigned evaluations = 99;
    if (ob_workspace(0, storage, 16, &sum, &evaluations) != -1 || sum != 77 ||
        evaluations != 99)
    {
        return 41;
    }
    if (ob_workspace(16, storage, 16, &sum, &evaluations) || sum != 136)
    {
        return 41;
    }
    printf("OB_EDITION:standard=%ld,evaluations=%u\n", (long)__STDC_VERSION__,
           evaluations);
    return 0;
}
