#ifndef SERIAL_RETRY_H
#define SERIAL_RETRY_H
#include "systems.h"
#include "serial_depth.h"
#include "timing.h"
#include "i2c.h"

enum
{
    SR_OK,
    SR_PENDING,
    SR_READY,
    SR_DONE,
    SR_INVALID,
    SR_AMBIGUOUS,
    SR_TIMEOUT,
    SR_LOST,
    SR_NACK,
    SR_UNCERTAIN,
    SR_EXHAUSTED,
    SR_STALE,
    SR_UNIMPLEMENTED
};

struct sr_timer
{
    unsigned bits;
    uint32_t last, remaining;
};

int sr_timer_begin(struct sr_timer *, unsigned, uint32_t, uint32_t);
int sr_timer_step(struct sr_timer *, uint32_t, uint32_t *);

struct sr_byte
{
    struct sr_timer timer;
    struct s_si2c bit;
    uint32_t stretch, low_ticks;
    unsigned low, terminal;
};

int sr_byte_begin(struct sr_byte *, unsigned, unsigned, uint32_t, uint32_t,
                  uint32_t);
int sr_byte_sample(struct sr_byte *, uint32_t, unsigned, unsigned, unsigned);

struct sr_txn
{
    struct sr_timer timer;
    struct sd_sdi2c bus;
    struct s_si2c bit;
    uint32_t token;
    uint8_t address, wire;
    unsigned action, terminal;
};

int sr_txn_begin(struct sr_txn *, uint32_t, uint8_t, uint8_t, unsigned,
                 uint32_t, uint32_t);
int sr_txn_bit(struct sr_txn *, uint32_t, uint32_t, unsigned, unsigned);
int sr_txn_read(struct sr_txn *, uint32_t, uint32_t, uint8_t, uint8_t *);

struct sr_retry
{
    struct sr_timer timer;
    uint32_t token, free_ticks, wait_ticks, bus_free, backoff;
    unsigned command, maximum, attempts, state, free;
};

int sr_retry_begin(struct sr_retry *, unsigned, uint32_t, uint32_t, uint32_t,
                   unsigned, unsigned, uint32_t, uint32_t);
int sr_retry_poll(struct sr_retry *, uint32_t, unsigned, unsigned, unsigned);
int sr_retry_complete(struct sr_retry *, uint32_t, uint32_t, unsigned,
                      unsigned);
#endif
