#ifndef SERIAL_I2C_FOLLOWTHROUGH_H
#define SERIAL_I2C_FOLLOWTHROUGH_H
#include <stdint.h>

enum
{
    IA_OK,
    IA_INVALID,
    IA_EMPTY,
    IA_UNSUPPORTED,
    IA_UNIMPLEMENTED
};

struct ia_interval
{
    uint32_t minimum, maximum;
};

int ia_nominal(uint32_t vdd_mv, uint32_t vol_mv, uint32_t iol_ua,
               uint32_t rise_ns, uint32_t cap_pf, struct ia_interval *);

struct ia_envelope
{
    uint32_t vmin_mv, vmax_mv, vol_mv, iol_ua, rise_ns, cap_pf;
    uint32_t source_leak_ua, sink_leak_ua, tolerance_ppm;
};

int ia_guaranteed(const struct ia_envelope *, struct ia_interval *);

/* 0=pull-low,1=release; START2/STOP3 are deliberately outside data arbitration. */
struct ia_trace
{
    uint8_t bus[32], lost[32], active[32], survivors;
    unsigned low, high;
};

int ia_arbitrate(const uint8_t streams[3][32], unsigned controllers,
                 unsigned bits, const unsigned low[3], const unsigned high[3],
                 struct ia_trace *);
#endif
