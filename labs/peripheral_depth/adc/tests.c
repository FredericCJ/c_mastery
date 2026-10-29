#include "adc.h"
#ifndef PA_BIG_ENDIAN
#include <stdio.h>
#endif
#ifdef PA_BIG_ENDIAN
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#endif
#define CHECK(x)                                         \
    do                                                   \
    {                                                    \
        if (!(x))                                        \
        {                                                \
            printf("PA_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                    \
        }                                                \
    } while (0)

struct pa_model
{
    uint32_t r[10];
    unsigned reads, writes, late_overrun;
};

static uint32_t read_reg(void *p, unsigned r)
{
    struct pa_model *m = p;
    if (r >= 10)
    {
        return 0;
    }
    unsigned v = m->r[r];
    if (r == PA_DR)
    {
        m->reads++;
        m->r[PA_SR] &= ~PA_EOC;
        if (m->late_overrun)
        {
            m->r[PA_SR] |= PA_OVR;
        }
    }
    return v;
}

static void write_reg(void *p, unsigned r, uint32_t v)
{
    struct pa_model *m = p;
    if (r >= 10)
    {
        return;
    }
    m->writes++;
    if (r == PA_SR)
    {
        m->r[r] &= v;
    }
    else
    {
        m->r[r] = v;
    }
}

static struct pa_io io_for(struct pa_model *m)
{
    struct pa_io io = {m, read_reg, write_reg};
    return io;
}

static void complete(struct pa_model *m, unsigned v)
{
    if (m->r[PA_SR] & PA_EOC)
    {
        m->r[PA_SR] |= PA_OVR;
    }
    m->r[PA_SR] |= PA_EOC;
    m->r[PA_DR] = v;
}

int main(void)
{
    struct pu_clock clock = {64000000, 1, 1, 2, 2, 0};
    struct pa_config cfg = {4, 1, 0};
    struct pa_timing t;
    CHECK(pa_timing(&clock, &cfg, &t) == PA_OK && t.hz == 16000000 &&
          t.total_cycles == 96 && t.conversion_us == 6);
    struct pu_clock alternate = {76000000, 1, 1, 2, 1, 0};
    CHECK(pa_timing(&alternate, &cfg, &t) == PA_OK && t.hz == 9500000 &&
          t.prescaler == 3);
    cfg.channels = 2;
    CHECK(pa_timing(&clock, &cfg, &t) == PA_OK && t.total_cycles == 192 &&
          t.conversion_us == 12);
    cfg.sample = 8;
    CHECK(pa_timing(&clock, &cfg, &t) == PA_INVALID);
    cfg.sample = 4;
    for (unsigned wrap = 0; wrap < 2; wrap++)
    {
        for (unsigned n = 1; n <= 2; n++)
        {
            for (unsigned timer = 0; timer < 2; timer++)
            {
                struct pa_model m = {0};
                struct pa_adc s = {0};
                struct pa_io io = io_for(&m);
                uint32_t start = wrap ? UINT32_MAX - 1 : 0;
                uint16_t out[2] = {77, 88};
                cfg.channels = n;
                cfg.timer = timer;
                CHECK(pa_open(&s, io, &clock, &cfg, start) == PA_OK);
                CHECK(m.r[PA_SQR1] == (n - 1) * 1048576 &&
                      m.r[PA_SQR3] == (n == 2 ? 32u : 0u) &&
                      m.r[PA_CR1] == (n == 2 ? PA_SCAN : 0));
                CHECK(pa_ready(&s, start) == PA_BUSY &&
                      pa_ready(&s, start + 3) == PA_BUSY &&
                      pa_ready(&s, start + 4) == PA_OK);
                CHECK(pa_begin(&s, io, 2, 1, out, start + 4, 100) == PA_STALE);
                CHECK(pa_begin(&s, io, 1, 1, out, start + 4, 100) == PA_OK);
                CHECK(m.r[PA_CR2] ==
                      (PA_ADON | PA_EOCS | (timer ? PA_EXTERNAL : PA_SWSTART)));
                CHECK(pa_step(&s, io, 2, start + 4) == PA_STALE);
                for (unsigned i = 0; i < n; i++)
                {
                    complete(&m, 0xf000u + 100 + i);
                    CHECK(pa_step(&s, io, 1, start + 5 + i) ==
                          (i + 1 == n ? PA_OK : PA_BUSY));
                    if (i + 1 < n)
                    {
                        CHECK(out[0] == 77 && out[1] == 88);
                    }
                }
                CHECK(out[0] == 100 && out[1] == (n == 2 ? 101 : 88) &&
                      m.reads == n && !m.r[PA_CR2]);
                CHECK(pa_open(&s, io, &clock, &cfg, start + 20) == PA_OK &&
                      pa_ready(&s, start + 20) == PA_BUSY &&
                      pa_ready(&s, start + 24) == PA_OK);
                CHECK(pa_begin(&s, io, 1, 1, out, start + 24, 10) == PA_STALE);
                CHECK(pa_begin(&s, io, 1, 2, out, start + 24, 10) == PA_OK);
                out[0] = 77;
                out[1] = 88;
                if (n == 2)
                {
                    complete(&m, 99);
                    CHECK(pa_step(&s, io, 2, start + 25) == PA_BUSY);
                }
                CHECK(pa_step(&s, io, 2, start + 34) == PA_TIMEOUT &&
                      out[0] == 77 && out[1] == 88 && !m.r[PA_CR2]);
            }
        }
    }
    for (unsigned late = 0; late < 2; late++)
    {
        struct pa_model m = {0};
        struct pa_adc s = {0};
        struct pa_io io = io_for(&m);
        uint16_t out[2] = {77, 88};
        cfg.channels = 1;
        cfg.timer = 1;
        CHECK(pa_open(&s, io, &clock, &cfg, 0) == PA_OK &&
              pa_ready(&s, 0) == PA_BUSY && pa_ready(&s, 4) == PA_OK &&
              pa_begin(&s, io, 1, 1, out, 4, 20) == PA_OK);
        complete(&m, 42);
        if (late)
        {
            m.late_overrun = 1;
        }
        else
        {
            complete(&m, 43);
        }
        CHECK(pa_step(&s, io, 1, 5) == PA_OVERRUN && out[0] == 77 &&
              !m.r[PA_CR2]);
    }
    {
        struct pa_model m = {0};
        struct pa_adc s = {0};
        struct pa_io io = io_for(&m);
        uint16_t out[2] = {77, 88};
        CHECK(pa_open(&s, io, &clock, &cfg, 0) == PA_OK &&
              pa_ready(&s, 0) == PA_BUSY && pa_ready(&s, 4) == PA_OK &&
              pa_begin(&s, io, 1, UINT32_MAX, out, 4, 20) == PA_OK);
        CHECK(pa_cancel(&s, io, UINT32_MAX) == PA_TIMEOUT && out[0] == 77);
        CHECK(pa_open(&s, io, &clock, &cfg, 5) == PA_OK &&
              pa_ready(&s, 5) == PA_BUSY && pa_ready(&s, 9) == PA_OK &&
              pa_begin(&s, io, 1, 1, out, 9, 20) == PA_STALE);
    }
    int32_t mv = 99;
    CHECK(pa_calibrate(1, 0, 3, 0, 10, &mv) == PA_OK && mv == 3);
    CHECK(pa_calibrate(2, 0, 3, 0, 10, &mv) == PA_OK && mv == 7);
    CHECK(pa_calibrate(0, 1, 3, 0, 3, &mv) == PA_OK && mv == -2);
    CHECK(pa_calibrate(42, 2, 2, 0, 10, &mv) == PA_INVALID && mv == -2);
    unsigned code = 88;
    CHECK(pa_ramp(0, 5, 512, 3584, &code) == PA_OK && code == 512);
    CHECK(pa_ramp(4, 5, 512, 3584, &code) == PA_OK && code == 3584);
    CHECK(pa_ramp(5, 5, 512, 3584, &code) == PA_INVALID && code == 3584);
    unsigned state = 0;
    CHECK(pa_threshold(&state, 101, 90, 100) == PA_OK && state == 1);
    CHECK(pa_threshold(&state, 95, 90, 100) == PA_OK && state == 1);
    CHECK(pa_threshold(&state, 90, 90, 100) == PA_OK && !state);
    puts("PA_PASS");
    return 0;
}
