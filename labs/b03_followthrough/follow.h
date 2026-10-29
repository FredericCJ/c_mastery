#ifndef B03_FOLLOW_H
#define B03_FOLLOW_H
#include "depth.h"

/* Version 1: see CONTRACTS.md; all non-atomic models require one owner. */
struct bf_round
{
    struct b3_writer writer[2];
    unsigned next;
};

int bf_turn(struct bf_round *, b3_write_fn callbacks[2], void *contexts[2],
            unsigned quota, unsigned *selected);

struct bf_message
{
    unsigned id, length, done, complete, effects;
    unsigned char request[6];
    unsigned total;
};

int bf_message(struct bf_message *, unsigned id, const unsigned char *,
               unsigned length, unsigned quota, int lose_reply,
               unsigned *result);

enum bf_phase
{
    BF_IDLE,
    BF_ACTIVE,
    BF_REQUESTED,
    BF_QUEUED,
    BF_DONE,
    BF_CANCELLED
};

enum bf_event
{
    BF_TIMEOUT,
    BF_REJECT,
    BF_QUEUED_ACK,
    BF_COMPLETE,
    BF_QUIESCENT
};

struct bf_operation
{
    unsigned generation, phase, owned, reports;
};

int bf_start(struct bf_operation *);
int bf_event(struct bf_operation *, unsigned generation, enum bf_event);

/* 0 empty, 1 producer owns, 2 published, 3 consumer owns. */
struct bf_box
{
    atomic_uint state;
    int payload;
};

void bf_box_init(struct bf_box *);
int bf_put(struct bf_box *, int);
int bf_get(struct bf_box *, int *);

struct bf_register
{
    unsigned status, reads, writes, last_write;
};

int bf_clear_read(struct bf_register *, unsigned *);
int bf_w0c(struct bf_register *, unsigned mask);
typedef unsigned (*bf_read_half_fn)(void *, unsigned high);
int bf_coherent(bf_read_half_fn, void *, unsigned attempts, uint16_t *);
int bf_due16(uint16_t now, uint16_t start, uint16_t delay, int *);
int bf_ticks(unsigned milliseconds, unsigned tick_numerator,
             unsigned tick_denominator, unsigned *ticks);
#endif
