#ifndef PL_COMPARE_H
#define PL_COMPARE_H
#include "service.h"
typedef uint32_t (*pl_tick_fn)(void *);

struct pl_byte_port
{
    volatile uint32_t *status, *data;
    uint32_t txe, tc;
    pl_tick_fn tick;
    void *context;
};

/* Selected preconfigured 8N1 one-byte operation, timeout in progressing ticks.
 * On TIMEOUT, memory is no longer borrowed but the line may still be owned. */
int pl_byte(struct pl_byte_port, uint8_t, uint32_t timeout);
struct pl_byte_port pl_direct_f439(volatile uint32_t *, pl_tick_fn, void *);
struct pl_byte_port pl_direct_g474(volatile uint32_t *, pl_tick_fn, void *);
struct pl_byte_port pl_generated(volatile uint32_t *, unsigned revision,
                                 pl_tick_fn, void *);
#endif
