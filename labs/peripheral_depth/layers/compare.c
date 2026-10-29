#include "compare.h"

int pl_byte(struct pl_byte_port p, uint8_t b, uint32_t t)
{
    (void)p;
    (void)b;
    (void)t;
    return PL_UNIMPLEMENTED;
}

struct pl_byte_port pl_direct_f439(volatile uint32_t *p, pl_tick_fn t, void *c)
{
    (void)p;
    (void)t;
    (void)c;
    return (struct pl_byte_port){0};
}

struct pl_byte_port pl_direct_g474(volatile uint32_t *p, pl_tick_fn t, void *c)
{
    (void)p;
    (void)t;
    (void)c;
    return (struct pl_byte_port){0};
}

struct pl_byte_port pl_generated(volatile uint32_t *p, unsigned r, pl_tick_fn t,
                                 void *c)
{
    (void)p;
    (void)r;
    (void)t;
    (void)c;
    return (struct pl_byte_port){0};
}
