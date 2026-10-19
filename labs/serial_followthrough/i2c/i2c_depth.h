#ifndef SERIAL_I2C_FOLLOWTHROUGH_H
#define SERIAL_I2C_FOLLOWTHROUGH_H
#include <stdint.h>
enum { IA_OK,IA_INVALID,IA_EMPTY,IA_UNSUPPORTED,IA_UNIMPLEMENTED };
typedef struct {uint32_t minimum,maximum;} IaInterval;
int ia_nominal(uint32_t vdd_mv,uint32_t vol_mv,uint32_t iol_ua,
               uint32_t rise_ns,uint32_t cap_pf,IaInterval *);
typedef struct {
 uint32_t vmin_mv,vmax_mv,vol_mv,iol_ua,rise_ns,cap_pf;
 uint32_t source_leak_ua,sink_leak_ua,tolerance_ppm;
} IaEnvelope;
int ia_guaranteed(const IaEnvelope *,IaInterval *);
/* 0=pull-low,1=release; START2/STOP3 are deliberately outside data arbitration. */
typedef struct {uint8_t bus[32],lost[32],active[32],survivors;unsigned low,high;} IaTrace;
int ia_arbitrate(const uint8_t streams[3][32],unsigned controllers,unsigned bits,
                 const unsigned low[3],const unsigned high[3],IaTrace *);
#endif
