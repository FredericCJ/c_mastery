#ifndef ETHERNET_PHY_MODEL_H
#define ETHERNET_PHY_MODEL_H
#include "phy.h"

struct ep_model
{
    uint16_t phy[32][32], data;
    uint32_t ar, tick;
    unsigned calls, writes, reads, transfers, busy_reads, delay, stuck, frozen,
        latch, reset_reads, violation, fail_at;
    unsigned reset_value, restart_value, restart_reads;
};

int ep_model_read(void *, unsigned, uint32_t *);
int ep_model_write(void *, unsigned, uint32_t);
uint32_t ep_model_clock(void *);
struct ed_bus ep_model_bus(struct ep_model *);
void ep_model_setup(struct ep_model *, unsigned);
#endif
