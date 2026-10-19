#ifndef ETHERNET_RING_MODEL_H
#define ETHERNET_RING_MODEL_H
#include "ring.h"
typedef struct {
 EnMemory *memory;uint32_t base,reg[1032];
 unsigned calls,fail_at,writes,settled,fences,violation,reset_reads,reset_stuck,stop_stuck;
 unsigned tx_current,rx_current,tx_demands,rx_demands,wire_length;
 unsigned char wire[1518];
} EnModel;
void en_model_init(EnModel *,EnMemory *,uint32_t);
EnBus en_model_bus(EnModel *);
int en_model_tx(EnModel *,unsigned error);
int en_model_rx(EnModel *,const void *,size_t,uint32_t status);
#endif
