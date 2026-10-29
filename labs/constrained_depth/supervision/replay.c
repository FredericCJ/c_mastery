#include "supervision.h"
#include <stdio.h>

int main(void)
{
    for (unsigned tasks = 2; tasks <= 3; ++tasks)
        for (unsigned word = 0; word < 4096; ++word)
        {
            struct cs_world w;
            if (cs_init(&w, tasks, 1) != CS_OK)
                return 1;
            unsigned x = word, bits = 0;
            for (unsigned step = 0; step < 6; ++step)
            {
                unsigned op = x & 3;
                x >>= 2;
                if (op == 3)
                {
                    enum cs_status s = cs_window(&w);
                    if (s == CS_OK)
                        bits |= 1u << step;
                    else if (s != CS_WAIT)
                        return 1;
                }
                else if (op < tasks && cs_progress(&w, op) != CS_OK)
                    return 1;
            }
            printf("%u %u %u", tasks, word, bits);
            for (unsigned i = 0; i < tasks; ++i)
                printf(" %u", w.last[i]);
            puts("");
        }
    return 0;
}
