#ifndef B03_FOLLOW_H
#define B03_FOLLOW_H
#include "depth.h"
/* Version 1: see CONTRACTS.md; all non-atomic models require one owner. */
typedef struct { B3Writer writer[2]; unsigned next; } BfRound;
int bf_turn(BfRound *, B3Write callbacks[2], void *contexts[2], unsigned quota,
            unsigned *selected);
typedef struct {
 unsigned id, length, done, complete, effects;
 unsigned char request[6]; unsigned total;
} BfMessage;
int bf_message(BfMessage *, unsigned id, const unsigned char *, unsigned length,
               unsigned quota, int lose_reply, unsigned *result);
enum BfPhase { BF_IDLE, BF_ACTIVE, BF_REQUESTED, BF_QUEUED, BF_DONE, BF_CANCELLED };
enum BfEvent { BF_TIMEOUT, BF_REJECT, BF_QUEUED_ACK, BF_COMPLETE, BF_QUIESCENT };
typedef struct { unsigned generation, phase, owned, reports; } BfOperation;
int bf_start(BfOperation *);
int bf_event(BfOperation *, unsigned generation, enum BfEvent);
/* 0 empty, 1 producer owns, 2 published, 3 consumer owns. */
typedef struct { atomic_uint state; int payload; } BfBox;
void bf_box_init(BfBox *);
int bf_put(BfBox *, int);
int bf_get(BfBox *, int *);
typedef struct { unsigned status, reads, writes, last_write; } BfRegister;
int bf_clear_read(BfRegister *, unsigned *);
int bf_w0c(BfRegister *, unsigned mask);
typedef unsigned (*BfReadHalf)(void *, unsigned high);
int bf_coherent(BfReadHalf, void *, unsigned attempts, uint16_t *);
int bf_due16(uint16_t now, uint16_t start, uint16_t delay, int *);
int bf_ticks(unsigned milliseconds, unsigned tick_numerator,
             unsigned tick_denominator, unsigned *ticks);
#endif
