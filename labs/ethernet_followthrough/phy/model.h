#ifndef ETHERNET_PHY_MODEL_H
#define ETHERNET_PHY_MODEL_H
#include "phy.h"
typedef struct {
 uint16_t phy[32][32],data;
 uint32_t ar,tick;
 unsigned calls,writes,reads,transfers,busy_reads,delay,stuck,frozen,latch,reset_reads,violation,fail_at;
 unsigned reset_value,restart_value,restart_reads;
} EpModel;
int ep_model_read(void *,unsigned,uint32_t *);
int ep_model_write(void *,unsigned,uint32_t);
uint32_t ep_model_clock(void *);
EDBus ep_model_bus(EpModel *);
void ep_model_setup(EpModel *,unsigned);
#endif
