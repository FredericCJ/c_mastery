#include "mmio.h"
#include <stdio.h>

int main(void)
{
    for (unsigned initial = 0; initial < 32; ++initial)
        for (unsigned acknowledge = 0; acknowledge < 32; ++acknowledge)
        {
            struct pm_model m = {0};
            m.revision = 1;
            m.status = initial;
            uint32_t v = acknowledge;
            if (pm_access(&m, PM_STATUS, 32, 1, &v))
                return 1;
            printf("W %u %u %u\n", initial, acknowledge, m.status);
        }
    for (unsigned initial = 0; initial < 16; ++initial)
        for (unsigned set = 0; set < 16; ++set)
            for (unsigned clear = 0; clear < 16; ++clear)
            {
                struct pm_model m = {0};
                m.revision = 1;
                m.odr = initial;
                uint32_t v = set | (clear << 16);
                if (pm_access(&m, PM_BSRR, 32, 1, &v))
                    return 1;
                printf("B %u %u %u %u\n", initial, set, clear, m.odr);
            }
    for (unsigned mode = 0; mode < 2; ++mode)
        for (unsigned seq = 0; seq < 256; ++seq)
            for (unsigned threshold = 1; threshold <= 4; ++threshold)
            {
                struct pm_filter d;
                unsigned changed;
                if (pm_filter_init(&d, threshold, mode ? 5 : 0, 4))
                    return 1;
                uint32_t tick = UINT32_MAX - 9;
                for (unsigned i = 0; i < 8; ++i)
                {
                    tick += i % 3 + 1;
                    if (pm_sample(&d, (seq >> i) & 1, tick, &changed))
                        return 1;
                    printf("F %u %u %u %u %u %u %u %u\n", mode, seq, threshold,
                           i, tick, d.stable, d.events, changed);
                }
            }
    return 0;
}
