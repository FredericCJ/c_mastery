/*
 * Edition probe: built under several -std= modes; each build records the
 * standard version the compiler reports. Exit status 0 when
 * ob_workspace behaves as specified, 41 otherwise.
 */
#include "buffer.h"
#include <stdio.h>

enum
{
    STORAGE_ELEMENTS = 16,
    SUM_OF_1_TO_16 = 16 * 17 / 2
};

int main(void)
{
    int storage[STORAGE_ELEMENTS];
    int sum = 77; /* sentinel: must survive a rejected request */
    unsigned evaluations = 99;
    if (ob_workspace(0, storage, STORAGE_ELEMENTS, &sum, &evaluations) != -1 ||
        sum != 77 || evaluations != 99)
    {
        return 41;
    }
    if (ob_workspace(STORAGE_ELEMENTS, storage, STORAGE_ELEMENTS, &sum,
                     &evaluations) != 0 ||
        sum != SUM_OF_1_TO_16)
    {
        return 41;
    }
    printf("OB_EDITION:standard=%ld,evaluations=%u\n", (long)__STDC_VERSION__,
           evaluations);
    return 0;
}
