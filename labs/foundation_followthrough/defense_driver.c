#include "depth.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    unsigned length, authority;
    while (scanf("%u %u", &length, &authority) == 2)
    {
        int32_t counter;
        unsigned char bytes[9];
        if (length > 9 || authority > 1 || scanf("%" SCNd32, &counter) != 1)
        {
            return 2;
        }
        for (unsigned i = 0; i < length; ++i)
        {
            unsigned byte;
            if (scanf("%u", &byte) != 1 || byte > 255)
            {
                return 2;
            }
            bytes[i] = (unsigned char)byte;
        }
        enum fd_status status =
            fd_apply(bytes, length, (int)authority, &counter);
        printf("%d %" PRId32 "\n", status, counter);
    }
    return 0;
}
