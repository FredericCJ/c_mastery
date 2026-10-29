#include "physical.h"

int ep_master_poll_rx(struct ep_master *s, uint32_t n, struct edu_frame *o)
{
    (void)s;
    (void)n;
    (void)o;
    return EP_STUB;
}

int ep_slave_poll_rx(struct ep_slave *s, uint32_t n)
{
    (void)s;
    (void)n;
    return EP_STUB;
}

int ep_slave_receive(struct ep_slave *s, unsigned t, uint8_t v, uint32_t n)
{
    (void)s;
    (void)t;
    (void)v;
    (void)n;
    return EP_STUB;
}

int ep_reply(struct ep_slave *s, uint32_t t, const struct edu_frame *f,
             uint32_t n)
{
    (void)s;
    (void)t;
    (void)f;
    (void)n;
    return EP_STUB;
}

int ep_open(struct ep_tx *s, struct r5_io io, const struct pu_clock *c,
            uint32_t b, const struct r5_timing *t, uint32_t now)
{
    (void)s;
    (void)io;
    (void)c;
    (void)b;
    (void)t;
    (void)now;
    return EP_STUB;
}

int ep_fits(const struct ep_tx *s, unsigned n, uint32_t b, unsigned *out)
{
    (void)s;
    (void)n;
    (void)b;
    (void)out;
    return EP_STUB;
}

int ep_send(struct ep_tx *s, uint32_t t, const uint8_t *p, size_t n,
            uint32_t now, uint32_t b)
{
    (void)s;
    (void)t;
    (void)p;
    (void)n;
    (void)now;
    (void)b;
    return EP_STUB;
}

int ep_step(struct ep_tx *s, uint32_t t, uint32_t now)
{
    (void)s;
    (void)t;
    (void)now;
    return EP_STUB;
}

int ep_cancel(struct ep_tx *s, uint32_t t, uint32_t now)
{
    (void)s;
    (void)t;
    (void)now;
    return EP_STUB;
}

int ep_poll(struct ep_master *s, uint32_t t, uint32_t now)
{
    (void)s;
    (void)t;
    (void)now;
    return EP_STUB;
}

int ep_tick(struct ep_master *s, uint32_t now)
{
    (void)s;
    (void)now;
    return EP_STUB;
}

int ep_receive(struct ep_master *s, unsigned st, uint8_t v, uint32_t now,
               struct edu_frame *o)
{
    (void)s;
    (void)st;
    (void)v;
    (void)now;
    (void)o;
    return EP_STUB;
}

int ep_disconnect(struct ep_master *s, uint32_t now)
{
    (void)s;
    (void)now;
    return EP_STUB;
}
