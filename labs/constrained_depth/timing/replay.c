#include "timing.h"
#include <stdio.h>

static void row(unsigned bits, uint32_t now, uint32_t deadline)
{
    unsigned result = 99;
    int status = ct_order(bits, now, deadline, &result);
    printf("O,%u,%u,%u,%d,%u\n", bits, now, deadline, status, result);
}

int main(void)
{
    for (unsigned now = 0; now < 256; now++)
        for (unsigned deadline = 0; deadline < 256; deadline++)
            row(8, now, deadline);
    const uint32_t boundary[] = {0, 1, 200, 32767, 32768, 65534, 65535};
    for (unsigned i = 0; i < 7; i++)
        for (unsigned j = 0; j < 7; j++)
            row(16, boundary[i], boundary[j]);
    return 0;
}
