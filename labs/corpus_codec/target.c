#include "codec.h"
volatile unsigned cc_observed;

void cc_target(void)
{
    uint8_t input[8] = {1, 2, 1, 2, 1, 2, 1, 2}, frame[CC_FRAME_LIMIT],
            output[8];
    size_t n = 0, k = 0;
    int status = cc_compress(input, 8, frame, sizeof frame, &n);
    if (status == CC_OK)
        status = cc_expand(frame, n, output, sizeof output, &k);
    cc_observed = (unsigned)status + (unsigned)k;
    for (;;)
    {
    }
}
