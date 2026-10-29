#ifndef SERIAL_SPI_SCHEDULER_H
#define SERIAL_SPI_SCHEDULER_H
#include "spi.h"
#include "algo.h"

struct ss_slot
{
    uint32_t token;
    unsigned state, count, dma, budget, setup, hold, result;
    uint8_t tx[16], rx[16];
    uint16_t tx_words[16], rx_words[16];
};

struct ss_scheduler
{
    struct ps_spi driver;
    algo_ring pending;
    struct ss_slot slots[2];
    uint32_t last_token;
    unsigned initialized, active;
};

struct ss_result
{
    unsigned status, sent, received;
};

typedef int (*ss_submit_fn)(struct ps_spi *, struct ps_io, uint32_t, uint32_t,
                            const uint8_t *, uint8_t *, const uint16_t *,
                            uint16_t *, unsigned, unsigned, uint32_t, unsigned,
                            unsigned, unsigned);

enum
{
    SS_FREE,
    SS_QUEUED,
    SS_RUNNING,
    SS_DONE
};

int ss_open(struct ss_scheduler *, struct ps_io, const struct pu_clock *,
            unsigned mode, unsigned divider);
int ss_enqueue(struct ss_scheduler *, uint32_t token, const uint8_t *,
               unsigned count, unsigned dma, unsigned budget, unsigned setup,
               unsigned hold);
int ss_run(struct ss_scheduler *, struct ps_io, ss_submit_fn, uint32_t now);
int ss_event(struct ss_scheduler *, struct ps_io, uint32_t token, uint32_t now);
int ss_cancel(struct ss_scheduler *, struct ps_io, uint32_t token);
int ss_reset(struct ss_scheduler *, struct ps_io, uint32_t token, uint32_t now);
int ss_take(struct ss_scheduler *, uint32_t token, struct ss_result *,
            uint8_t output[16]);
int ss_portable_submit(struct ps_spi *, struct ps_io, uint32_t, uint32_t,
                       const uint8_t *, uint8_t *, const uint16_t *, uint16_t *,
                       unsigned, unsigned, uint32_t, unsigned, unsigned,
                       unsigned);
int ss_f439_submit(struct ps_spi *, struct ps_io, uint32_t, uint32_t,
                   const uint8_t *, uint8_t *, const uint16_t *, uint16_t *,
                   unsigned, unsigned, uint32_t, unsigned, unsigned, unsigned);
#endif
