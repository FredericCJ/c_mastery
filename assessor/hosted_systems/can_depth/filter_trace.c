#include "frames.h"
#include <stdio.h>

int main(void)
{
    for (unsigned e = 0; e < 2; ++e)
        for (unsigned r = 0; r < 2; ++r)
        {
            struct can_filter f;
            if (ca_filter(0x123, e, r, &f))
                return 1;
            for (unsigned id = 0; id < 2048; ++id)
                for (unsigned got_e = 0; got_e < 2; ++got_e)
                    for (unsigned got_r = 0; got_r < 2; ++got_r)
                    {
                        struct ca_frame v = {.id = id,
                                             .extended = got_e,
                                             .kind =
                                                 got_r ? CA_REMOTE : CA_DATA};
                        printf("%u %u %u %u %u %d\n", e, r, id, got_e, got_r,
                               ca_accept(&v, &f, 0));
                    }
        }
}
