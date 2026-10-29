#ifndef AR_RESERVATION_H
#define AR_RESERVATION_H
#include "reclamation.h"
#define AR_CAPACITY 4u
#define AR_LIMIT ((UINT_MAX >> 3) - 1u)

enum
{
    AR_OK,
    AR_EMPTY,
    AR_PENDING,
    AR_FULL,
    AR_CANCELLED,
    AR_TOO_LATE,
    AR_INVALID,
    AR_EXHAUSTED,
    AR_UNIMPLEMENTED
};

struct ar_cell
{
    atomic_uint state;
    struct ap_value value;
};

struct ar_queue
{
    atomic_uint head, tail;
    struct ar_cell cells[AR_CAPACITY];
    ap_hook_fn hook;
    void *context;
};

struct ar_ticket
{
    const struct ar_queue *owner;
    unsigned sequence;
};

void ar_init(struct ar_queue *, ap_hook_fn, void *);
int ar_reserve(struct ar_queue *, struct ar_ticket *);
/* Exactly one producer calls finish once; cancellation does not replace it. */
int ar_finish(struct ar_queue *, struct ar_ticket, struct ap_value);
int ar_cancel(struct ar_queue *, struct ar_ticket);
int ar_pop(struct ar_queue *, struct ap_value *);
#endif
