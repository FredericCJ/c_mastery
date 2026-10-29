#include "clock.h"
#include <inttypes.h>
#include <stdio.h>

struct xi_counter
{
    uint64_t start, step, total_mask;
    unsigned bits, part;
};

static uint64_t point(struct xi_counter *c)
{
    return (c->start + (c->part++) * c->step) & c->total_mask;
}

static uint32_t high(void *p)
{
    struct xi_counter *c = p;
    return (uint32_t)(point(c) >> c->bits);
}

static uint32_t low(void *p)
{
    struct xi_counter *c = p;
    return (uint32_t)point(c) & ((1u << c->bits) - 1);
}

static uint64_t guard(void *p)
{
    return ((struct xi_counter *)p)->part;
}

int main(void)
{
    for (unsigned lb = 1; lb <= 4; ++lb)
    {
        for (unsigned hb = 1; hb <= 3; ++hb)
        {
            for (unsigned start = 0; start < (1u << (lb + hb)); ++start)
            {
                for (unsigned step = 0; step <= 4; ++step)
                {
                    struct xi_counter c = {start, step, (1u << (lb + hb)) - 1,
                                           lb, 0};
                    struct xk_snapshot out = {.value = 999};
                    int rc = xk_snapshot(
                        (struct xk_io){&c, high, low, guard},
                        (struct xk_policy){lb, hb, 1, 2 * step, 3}, &out);
                    printf("S %u %u %u %u %d %" PRIu64 " %u\n", lb, hb, start,
                           step, rc, out.value, c.part);
                }
            }
        }
    }
    uint32_t random = 0x58b49d23;
    for (unsigned i = 0; i < 4096; ++i)
    {
        random = random * 1664525u + 1013904223u;
        uint64_t anchor = (uint64_t)random << 24, begin = random & 1023,
                 end = begin + ((random >> 10) & 31);
        if (i % 17 == 0)
        {
            anchor = UINT64_MAX - (random & 1023);
        }
        uint32_t quantum =
                     i % 13 == 0 ? UINT32_MAX : ((random >> 12) & 65535) + 1,
                 ppm = random % 1000001u;
        struct xk_calibration c = {0, anchor, anchor, quantum, ppm, 7};
        struct xk_interval out = {999, 1000, 9};
        int rc = xk_calibrate(&c, begin, end, &out);
        printf("C %" PRIu64 " %" PRIu64 " %" PRIu64 " %u %u %d %" PRIu64
               " %" PRIu64 " %u\n",
               anchor, begin, end, quantum, ppm, rc, out.lower, out.upper,
               out.epoch);
    }
    uint32_t values[] = {0, 1, 2, UINT32_MAX - 1, UINT32_MAX};
    for (unsigned a = 0; a < 5; ++a)
    {
        for (unsigned b = 0; b < 5; ++b)
        {
            for (unsigned bound = 1; bound <= 4; ++bound)
            {
                for (unsigned epoch = 1; epoch <= 2; ++epoch)
                {
                    for (unsigned full = 0; full < 2; ++full)
                    {
                        struct xk_sequence seq = {1, values[a], 1,
                                                  full ? UINT64_MAX : 10};
                        int rc = xk_sequence(&seq, epoch, values[b], bound);
                        printf("G %u %u %u %u %u %d %u %" PRIu64 "\n",
                               values[a], values[b], bound, epoch, full, rc,
                               seq.last, seq.missing);
                    }
                }
            }
        }
    }
    for (unsigned al = 0; al < 8; ++al)
    {
        for (unsigned au = al; au < 8; ++au)
        {
            for (unsigned bl = 0; bl < 8; ++bl)
            {
                for (unsigned bu = bl; bu < 8; ++bu)
                {
                    unsigned out = 99;
                    int rc = xk_order((struct xk_interval){al, au, 1},
                                      (struct xk_interval){bl, bu, 1}, &out);
                    printf("O %u %u %u %u %d %u\n", al, au, bl, bu, rc, out);
                }
            }
        }
    }
    for (unsigned lower = 0; lower < 8; ++lower)
    {
        for (unsigned upper = lower; upper < 8; ++upper)
        {
            for (unsigned minimum = 0; minimum < 4; ++minimum)
            {
                for (unsigned maximum = minimum; maximum < 4; ++maximum)
                {
                    struct xk_interval out = {999, 1000, 9};
                    int rc = xk_event((struct xk_interval){lower, upper, 1},
                                      minimum, maximum, &out);
                    printf("E %u %u %u %u %d %" PRIu64 " %" PRIu64 " %u\n",
                           lower, upper, minimum, maximum, rc, out.lower,
                           out.upper, out.epoch);
                }
            }
        }
    }
}
