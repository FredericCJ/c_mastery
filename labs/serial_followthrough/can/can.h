#ifndef CAN_CURRENT_DEPTH_H
#define CAN_CURRENT_DEPTH_H
#include "serial_depth.h"

enum
{
    CN_OK,
    CN_INVALID,
    CN_MORE,
    CN_FRAME,
    CN_ERROR,
    CN_BUSY,
    CN_UNAVAILABLE,
    CN_UNIMPLEMENTED
};

struct cn_stream
{
    uint8_t raw[98];
    unsigned used, needed, total, run, last, expect, tail, ack, stage;
};

struct cn_decoded
{
    struct sd_can frame;
    unsigned ack;
};

/* Zero-initialize/reset only after abandoning an old prefix. Strict local
   standard data/DLC0..8 subset, not all permissible classic receive forms. */
int cn_feed(struct cn_stream *, const uint8_t *, size_t, size_t *,
            struct cn_decoded *);

struct cn_search
{
    struct sd_timing timing;
    unsigned candidates, sample_num, sample_den;
};

int cn_search(uint32_t clock, uint32_t rate, unsigned sample_num,
              unsigned sample_den, uint32_t minimum_bs1_ns, struct cn_search *);

enum
{
    CN_ACTIVE,
    CN_PASSIVE,
    CN_OFF
};

enum
{
    CN_ARBITRATION_LOSS,
    CN_RECEIVER_ERROR,
    CN_FIRST_DOMINANT,
    CN_TRANSMITTER_FLAG,
    CN_BIT_DURING_FLAG,
    CN_DOMINANT_RUN,
    CN_TRANSMIT_SUCCESS,
    CN_RECEIVE_SUCCESS,
    CN_RESTART,
    CN_RECOVERY_BIT
};

struct cn_event
{
    unsigned kind, transmitter, active_or_overload, ack_missing,
        passive_flag_all_recessive, prearbitration_stuff_exception, value;
};

struct cn_fault
{
    unsigned tec, rec, state, enabled, restart, idles, recessive;
};

struct cn_effect
{
    unsigned before_tec, before_rec, before_state, rules, after_tec, after_rec,
        after_state;
};

/* Explicit classified-MAC event model. Each complete episode appears once;
   caller establishes event premises. No bus decoder or physical FCE claim. */
int cn_fault(struct cn_fault *, const struct cn_event *, struct cn_effect *);

enum
{
    CN_MCR = 0,
    CN_MSR = 4,
    CN_TSR = 8,
    CN_RF0R = 12,
    CN_IER = 20,
    CN_ESR = 24,
    CN_BTR = 28,
    CN_TI0R = 0x180,
    CN_TDT0R = 0x184,
    CN_TDL0R = 0x188,
    CN_TDH0R = 0x18c,
    CN_RI0R = 0x1b0,
    CN_RDT0R = 0x1b4,
    CN_RDL0R = 0x1b8,
    CN_RDH0R = 0x1bc,
    CN_FMR = 0x200,
    CN_FM1R = 0x204,
    CN_FS1R = 0x20c,
    CN_FFA1R = 0x214,
    CN_FA1R = 0x21c,
    CN_F0R1 = 0x240,
    CN_F0R2 = 0x244
};

struct cn_io
{
    void *context;
    uint32_t (*read)(void *, unsigned);
    void (*write)(void *, unsigned, uint32_t);
};

enum
{
    CN_COLD,
    CN_ENTERING,
    CN_SYNCHRONIZING,
    CN_RUNNING,
    CN_CONTROL_FAILED
};

enum
{
    CN_SENT,
    CN_LOST,
    CN_TX_ERROR,
    CN_ABORTED
};

struct cn_controller
{
    struct cn_io io;
    unsigned state, nart, locked, remaining, limit, recovery, tx_state,
        cancelling, result;
    uint32_t btr, last_token, token;
};

struct cn_rx
{
    struct sd_can frame;
    unsigned overrun, filter, timestamp;
};

/* One foreground owner; actual CAN1/filter bank0 exclusive ownership. A fresh
   zero-initialized controller is required. Callbacks cannot reenter or fail. */
int cn_begin(struct cn_controller *, struct cn_io, const struct sd_timing *,
             unsigned, unsigned, unsigned);
int cn_control(struct cn_controller *);
int cn_recover(struct cn_controller *, unsigned);
int cn_send(struct cn_controller *, const struct sd_can *, uint32_t *);
int cn_tx_poll(struct cn_controller *, uint32_t);
int cn_cancel(struct cn_controller *, uint32_t);
int cn_take(struct cn_controller *, uint32_t, unsigned *);
int cn_receive(struct cn_controller *, struct cn_rx *);
int cn_ack_overrun(struct cn_controller *);

struct cn_queue
{
    struct cn_rx item[3];
    unsigned count, loss;
};

int cn_queue_put(struct cn_queue *, const struct cn_rx *);
int cn_queue_get(struct cn_queue *, struct cn_rx *);
struct cn_io cn_f439_io(void);
int cn_controller_test(unsigned);
int cn_test(unsigned);
#endif
