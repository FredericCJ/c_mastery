#include "timer.h"
#include <stdio.h>

int main(void)
{
    unsigned clocks[] = {8000000, 16000000}, pscs[] = {0, 15, 65535},
             hzs[] = {1, 3, 1000, 1001, 8000000, 16000000},
             duties[] = {0, 1, 250, 500, 999, 1000},
             ppms[] = {0, 1000, 1000000};
    for (unsigned a = 0; a < 2; a++)
        for (unsigned b = 0; b < 3; b++)
            for (unsigned h = 0; h < 6; h++)
                for (unsigned d = 0; d < 6; d++)
                    for (unsigned m = 0; m < 2; m++)
                        for (unsigned p = 0; p < 3; p++)
                        {
                            struct pu_clock k = {clocks[a], 1, 1, 1, 1, 0};
                            struct pt_config c = {0};
                            int rc =
                                pt_choose(&k, pscs[b], hzs[h], duties[d],
                                          1000 - duties[d], m, ppms[p], &c);
                            printf("C %u %u %u %u %u %u %d %u %u %u\n",
                                   clocks[a], pscs[b], hzs[h], duties[d], m,
                                   ppms[p], rc, c.arr, c.ccr[0], c.ccr[1]);
                        }
    for (unsigned m = 0; m < 2; m++)
        for (unsigned a = 1; a <= 16; a++)
            for (unsigned c = 0; c <= a + !m; c++)
            {
                struct pt_config v = {16000000, 1, 0, a, {c, c}, m};
                struct pt_model t;
                unsigned level;
                pt_init(&t, &v);
                for (unsigned i = 0; i < (m ? 2 * a : a + 1); i++)
                {
                    pt_level(&t, 0, &level);
                    printf("W %u %u %u %u %u %u %u\n", m, a, c, i, t.counter,
                           t.down, level);
                    pt_tick(&t);
                }
            }
    for (unsigned center = 0; center < 2; center++)
        for (unsigned word = 0; word < 3125; word++)
        {
            struct pt_config a = {16000000, 1, 1, 3, {1, 2}, center},
                             b = {16000000, 1, 2, 2, {0, 2}, center};
            struct pt_model t;
            unsigned n = word;
            pt_init(&t, &a);
            for (unsigned step = 0; step < 5; step++)
            {
                unsigned ev = n % 5;
                n /= 5;
                if (ev == 0)
                    pt_tick(&t);
                if (ev == 1)
                    pt_ug(&t);
                if (ev == 2)
                    t.udis = !t.udis;
                if (ev == 3)
                    t.urs = !t.urs;
                if (ev == 4)
                    pt_stage(&t, &b);
                printf("S %u %u %u %u %u %u %u %u %u %u %u %u %u\n", center,
                       word, step, t.counter, t.down, t.active.arr,
                       t.active.psc, t.active.ccr[0], t.active.ccr[1], t.udis,
                       t.urs, t.uif, t.updates);
            }
        }
    for (unsigned now = 0; now < 65536; now++)
        for (unsigned lost = 0; lost < 2; lost++)
        {
            unsigned v = 777;
            int r = pt_interval(65530, now, 32767, lost, &v);
            printf("I %u %u %d %u\n", now, lost, r, v);
        }
    return 0;
}
