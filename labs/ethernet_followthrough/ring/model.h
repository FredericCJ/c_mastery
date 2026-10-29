#ifndef ETHERNET_RING_MODEL_H
#define ETHERNET_RING_MODEL_H
#include "ring.h"

struct en_model
{
    struct en_memory *memory;
    uint32_t base, reg[1032];
    unsigned calls, fail_at, writes, settled, fences, violation, reset_reads,
        reset_stuck, stop_stuck;
    unsigned tx_current, rx_current, tx_demands, rx_demands, wire_length;
    unsigned char wire[1518];
};

void en_model_init(struct en_model *, struct en_memory *, uint32_t);
struct en_bus en_model_bus(struct en_model *);
int en_model_tx(struct en_model *, unsigned error);
int en_model_rx(struct en_model *, const void *, size_t, uint32_t status);
#endif
