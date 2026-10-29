#include "adc.h"
#include "firmware_io.h"
#include "timer.h"
#include <stdio.h>
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            printf("PA_COMPOSE_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                            \
        }                                                        \
    } while (0)

struct dg_m
{
    unsigned ready, code;
};

static uint32_t rd(void *p, unsigned r)
{
    struct dg_m *m = p;
    return r == FW_STATUS ? (m->ready ? FW_EOC : 0) : r == FW_ADC ? m->code : 0;
}

static void wr(void *p, unsigned r, uint32_t v)
{
    (void)p;
    (void)r;
    (void)v;
}

int main(void)
{
    struct dg_m m = {0, 0};
    struct fw_io io = {&m, rd, wr};
    uint16_t value = 99;
    CHECK(fw_adc(io, &value, 2) != 0 && value == 99);
    m.ready = 1;
    for (unsigned i = 0; i < 65536; i++)
    {
        m.code = i;
        CHECK(fw_adc(io, &value, 1) == 0 && value == (i % 4096));
    }
    struct pu_clock clock = {64000000, 1, 1, 2, 2, 0};
    struct pt_config timer;
    struct pa_config cfg = {4, 2, 1};
    struct pa_timing timing;
    CHECK(pt_choose(&clock, 63, 1000, 0, 0, 0, 0, &timer) == PT_OK &&
          timer.psc == 63 && timer.arr == 999 && timer.clock == 64000000);
    CHECK(pa_timing(&clock, &cfg, &timing) == PA_OK &&
          timing.total_cycles == 192 && timing.conversion_us == 12);
    struct pt_model model;
    CHECK(pt_init(&model, &timer) == PT_OK);
    model.running = 1;
    for (unsigned i = 0; i < 64000; i++)
        CHECK(pt_input(&model) == PT_OK);
    CHECK(model.updates == 1);
    puts("PA_COMPOSE_PASS65536codes1actualtimerperiod");
    return 0;
}
