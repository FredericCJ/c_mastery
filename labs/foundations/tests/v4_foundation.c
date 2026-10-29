#include "v4_foundation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "contract failure at observation %d\n", __LINE__); \
            return 1;                                                          \
        }                                                                      \
    } while (0)

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "diagnosis"))
    {
        for (uint32_t cap = 1; cap < 20; cap++)
        {
            struct v4_admission s = {cap, 0};
            CHECK(v4_admit(&s, cap + 1) < 0 && s.available == cap &&
                  s.active == 0);
            CHECK(v4_admit(&s, cap) == 0 && s.available == 0 &&
                  s.active == cap);
            CHECK(v4_release(&s, cap + 1) < 0 && s.active == cap);
            CHECK(v4_release(&s, cap) == 0 && s.available == cap &&
                  s.active == 0);
        }
        struct v4_admission s = {1, UINT32_MAX};
        CHECK(v4_admit(&s, 1) < 0 && s.available == 1);
    }
    else if (!strcmp(argv[1], "measurement"))
    {
        struct v4_sample a = {100, 50, 10, 20, 5, 1, UINT32_MAX - 4},
                         b = {200, 70, 15, 32, 11, 3, 5};
        struct v4_measurement o;
        CHECK(v4_measure(&a, &b, &o) == 0 && o.delta.wall_ns == 100 &&
              o.delta.cpu_ns == 20 && o.delta.cycles == 10 && o.cycle_wrap &&
              o.delta.write_accepted == 12 && o.delta.write_completed == 6 &&
              o.delta.read_bytes == 5 && o.delta.io_ops == 2);
        struct v4_measurement old = o;
        b.cpu_ns = 40;
        CHECK(v4_measure(&a, &b, &o) < 0 && !memcmp(&o, &old, sizeof o));
    }
    else
        return 2;
    return 0;
}
