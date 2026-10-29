#include "superloop.h"
#include <stdio.h>

int main(void)
{
    for (unsigned mode = 0; mode < 3; mode++)
    {
        for (unsigned word = 0; word < 4096; word++)
        {
            struct sc_schedule s;
            if (sc_schedule_init(&s, mode) != SC_OK)
            {
                return 1;
            }
            unsigned digits = word;
            printf("S,%u,%u", mode, word);
            for (unsigned t = 0; t < 4; t++)
            {
                unsigned ready = digits % 8;
                digits /= 8;
                if (mode)
                {
                    ready |= 8;
                }
                unsigned selected = 99;
                int status = sc_schedule_next(&s, t, ready, &selected);
                printf(",%d,%u", status, selected);
            }
            puts("");
        }
    }
    for (unsigned policy = 0; policy < 3; policy++)
    {
        for (unsigned period = 1; period <= 4; period++)
        {
            for (unsigned a = 0; a <= 6; a++)
            {
                for (unsigned b = a; b <= 6; b++)
                {
                    for (unsigned c = b; c <= 6; c++)
                    {
                        struct sc_releases r;
                        if (sc_releases_init(&r, period, 0, policy) != SC_OK)
                        {
                            return 1;
                        }
                        unsigned times[3] = {a, b, c};
                        printf("R,%u,%u,%u,%u,%u", policy, period, a, b, c);
                        for (unsigned i = 0; i < 3; i++)
                        {
                            int status = sc_releases_poll(&r, times[i]);
                            printf(
                                ",%d,%llu,%llu,%llu,%u,%llu,%llu", status,
                                (unsigned long long)r.next,
                                (unsigned long long)r.seen,
                                (unsigned long long)r.lost, r.count,
                                (unsigned long long)(r.count ? r.items[0] : 99),
                                (unsigned long long)(r.count > 1 ? r.items[1]
                                                                 : 99));
                        }
                        puts("");
                    }
                }
            }
        }
    }
    return 0;
}
