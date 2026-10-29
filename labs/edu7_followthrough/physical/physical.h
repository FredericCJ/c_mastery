#ifndef EDU7_PHYSICAL_V1_H
#define EDU7_PHYSICAL_V1_H
#include "channel.h"
#include "edu7_depth.h"
#include "foundation.h"

enum
{
    EP_OK,
    EP_INVALID,
    EP_BUSY,
    EP_STALE,
    EP_FULL,
    EP_TIMEOUT,
    EP_FRAME,
    EP_DROP,
    EP_STUB = 99
};

enum
{
    EP_IDLE,
    EP_ENABLE,
    EP_ACTIVE,
    EP_GUARD
};

struct ep_tx
{
    struct pu_driver byte;
    struct r5_io io;
    struct r5_delays delays;
    uint8_t bytes[96];
    uint32_t token, last, due, deadline, clock, divisor, first_deadline;
    unsigned phase, count, offset, chunk, result, cancel, enabled;
};

struct ep_master
{
    struct ep_tx tx;
    struct e7_grant grant;
    struct ef_framer parser;
    uint32_t token, last, echoes, faults, drops, delivered;
    unsigned polling;
};

struct ep_slave
{
    struct ep_tx tx;
    struct ef_framer parser;
    uint32_t last, origin;
    unsigned granted;
};

/* Fresh zeroed owner; fixed microsecond clock, nonwrapping campaign time. */
int ep_open(struct ep_tx *, struct r5_io, const struct pu_clock *, uint32_t,
            const struct r5_timing *, uint32_t);
int ep_fits(const struct ep_tx *, unsigned, uint32_t, unsigned *);
int ep_send(struct ep_tx *, uint32_t, const uint8_t *, size_t, uint32_t,
            uint32_t);
int ep_step(struct ep_tx *, uint32_t, uint32_t);
int ep_cancel(struct ep_tx *, uint32_t, uint32_t);
int ep_poll(struct ep_master *, uint32_t, uint32_t);
int ep_tick(struct ep_master *, uint32_t);
int ep_receive(struct ep_master *, unsigned, uint8_t, uint32_t,
               struct edu_frame *);
int ep_master_poll_rx(struct ep_master *, uint32_t, struct edu_frame *);
int ep_disconnect(struct ep_master *, uint32_t);
int ep_slave_receive(struct ep_slave *, unsigned, uint8_t, uint32_t);
int ep_slave_poll_rx(struct ep_slave *, uint32_t);
int ep_reply(struct ep_slave *, uint32_t, const struct edu_frame *, uint32_t);
int ep_f439_open(struct ep_tx *, const struct pu_clock *, uint32_t, unsigned,
                 uint32_t);
int ep_f439_open_port(struct ep_tx *, const struct pu_clock *, uint32_t,
                      unsigned, unsigned, uint32_t);
int ep_mapped(void);
int ep_tests(unsigned);
#endif
