#include "alternatives.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PA_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

int main(void)
{
    unsigned configurations = 0;
    const unsigned addresses[] = {0, 1, 4, 16, 252, 255, 256},
                   lengths[] = {0, 1, 4, 16, 64, 65},
                   times[] = {0, 1, 1000, 1001};
    for (unsigned a = 0; a < 7; a++)
    {
        for (unsigned l = 0; l < 6; l++)
        {
            for (unsigned t = 0; t < 4; t++)
            {
                struct pa_empty empty = {{0}};
                struct pa_address address;
                struct pa_length length;
                struct pa_timeout timeout;
                struct pt_builder b = {0};
                struct pt_config x = {91, 92, 93}, y = x;
                int one = pt_builder_set(&b, 0, addresses[a]),
                    two = pa_address(empty, addresses[a], &address);
                C(one == two);
                if (one)
                {
                    continue;
                }
                one = pt_builder_set(&b, 1, lengths[l]);
                two = pa_length(address, lengths[l], &length);
                C(one == two);
                if (one)
                {
                    continue;
                }
                one = pt_builder_set(&b, 2, times[t]);
                two = pa_timeout(length, times[t], &timeout);
                C(one == two);
                if (one)
                {
                    continue;
                }
                one = pt_builder_finish(&b, &x);
                two = pa_finish(timeout, &y);
                C(one == two && memcmp(&x, &y, sizeof x) == 0);
                configurations++;
            }
        }
    }
    for (unsigned pattern = 0; pattern < 256; pattern++)
    {
        struct pt_thread a = {0}, b = {0};
        uint32_t x = 91, y = 91;
        unsigned supplied = 0;
        for (unsigned tick = 0; tick < 8; tick++)
        {
            unsigned available = (pattern >> tick) & 1;
            uint32_t value = supplied + 1;
            int one = pt_thread_sum(&a, available, value, &x),
                two = pa_thread_sum(&b, available, value, &y);
            C(one == two && a.count == b.count && a.sum == b.sum && x == y);
            if (available && a.count < 3)
            {
                supplied++;
            }
        }
    }
    struct pt_thread t = {0, 2, UINT32_MAX};
    uint32_t out = 91;
    C(pa_thread_sum(&t, 1, 1, &out) == PT_RANGE && t.pc == 0 && t.count == 2 &&
      t.sum == UINT32_MAX && out == 91);
    for (unsigned n = 0; n <= 8; n++)
    {
        unsigned messages = 91;
        C(pa_session_pair(n, &messages) == 0 && messages == 3 + 2 * n);
    }
    printf(
        "PA_ALTERNATIVES:%u staged configurations;256 suspension traces;9 dual endpoint traces\n",
        configurations);
    return 0;
}
