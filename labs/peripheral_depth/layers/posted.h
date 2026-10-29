#ifndef PERIPHERAL_LAYER_POSTED_H
#define PERIPHERAL_LAYER_POSTED_H
#include "mmio.h"
#include "service.h"

struct pl_posted
{
    struct pm_model device;
    struct pm_access pending[8];
    unsigned head, count, immediate, delivered;
};

int pl_post_write(struct pl_posted *, unsigned, unsigned, uint32_t);
int pl_post_read(struct pl_posted *, unsigned, unsigned, uint32_t *);
int pl_flush(struct pl_posted *);

struct pl_exchange
{
    unsigned phase, left;
    uint8_t byte;
};

int pl_exchange_begin(struct pl_exchange *, struct pl_posted *, uint8_t,
                      unsigned);
int pl_exchange_step(struct pl_exchange *, struct pl_posted *, uint8_t *);
#endif
