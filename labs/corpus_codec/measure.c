#include "codec.h"
#include <stdio.h>
#include <time.h>

int main(void)
{
    uint8_t data[128], out[CC_FRAME_LIMIT];
    size_t used = 0;
    const char *names[] = {"equal", "periodic", "ramp", "permuted"};
    for (unsigned workload = 0; workload < 4; workload++)
    {
        for (unsigned i = 0; i < 128; i++)
        {
            data[i] = (uint8_t)(workload == 0   ? 7
                                : workload == 1 ? i % 3
                                : workload == 2 ? i
                                                : i * 31);
        }
        for (unsigned optimal = 0; optimal < 2; optimal++)
        {
            clock_t start = clock();
            unsigned checksum = 0;
            for (unsigned trial = 0; trial < 1000; trial++)
            {
                int status =
                    optimal
                        ? cc_compress_optimal(data, 128, out, sizeof out, &used)
                        : cc_compress(data, 128, out, sizeof out, &used);
                if (status != CC_OK)
                {
                    return 1;
                }
                checksum += (unsigned)used + out[trial % used];
            }
            clock_t finish = clock();
            if (start == (clock_t)-1 || finish == (clock_t)-1)
            {
                return 2;
            }
            printf(
                "%s %s input=128 frame=%zu trials=1000 cpu_ticks=%.0f clocks_per_sec=%ld checksum=%u\n",
                names[workload], optimal ? "optimal-token" : "greedy", used,
                (double)(finish - start), (long)CLOCKS_PER_SEC, checksum);
        }
    }
    return 0;
}
