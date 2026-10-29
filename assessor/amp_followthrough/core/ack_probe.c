#include "amp.h"
#include <stdio.h>

int main(void)
{
    struct af_ring incoming;
    af_ring_init(&incoming, 0);
    unsigned pending = 1;
    unsigned char bytes[32];
    size_t n = 99;
    uint32_t sequence = 99;
    /* Exact changed incarnation, deliberately reused sequence1. An ingress
       peer/wire record must pass epoch admission before queue publication. */
    struct af_message old = {.epoch = 1, .sequence = 1, .length = 0};
    struct af_message current = {.epoch = 2, .sequence = 1, .length = 0};
    if (af_batch(&incoming, 2, &old, 1) != AF_INVALID ||
        af_queued(&incoming) != 0)
    {
        return 1;
    }
    if (af_receive(&incoming, 2, bytes, 32, &sequence, &n) != AF_EMPTY ||
        !pending || sequence != 99)
    {
        return 1;
    }
    if (af_batch(&incoming, 2, &current, 1) != AF_OK)
    {
        return 1;
    }
    if (af_receive(&incoming, 2, bytes, 32, &sequence, &n) == AF_OK &&
        sequence == 1)
    {
        pending = 0;
    }
    if (pending || sequence != 1 || n != 0)
    {
        return 1;
    }
    puts("AF_PASS:ACK(1,1) refused before ACK(2,1) completes reused sequence1");
    return 0;
}
