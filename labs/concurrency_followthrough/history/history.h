#ifndef CF_HISTORY_H
#define CF_HISTORY_H
#include <stdint.h>

enum
{
    CH_FIFO,
    CH_STACK
};

enum
{
    CH_INSERT,
    CH_REMOVE
};

enum
{
    CH_VALUE,
    CH_FULL,
    CH_EMPTY
};

enum
{
    CH_VALID,
    CH_NONLINEAR,
    CH_INVALID,
    CH_PENDING,
    CH_LIMIT,
    CH_UNIMPLEMENTED = 99
};

struct ch_op
{
    uint32_t invoke, respond, invoke_event, respond_event;
    unsigned participant, operation, value, result, pending;
};

struct ch_witness
{
    unsigned count, selected_pending, order[10], result[10], value[10];
    uint32_t visited;
};

/* Initially empty, capacity1..8,0..10 calls,0..9 participants. Unique ordinal
 * events are required only in event_mode. Budget1..1000000 search nodes;
 * reaching it returns LIMIT, never an unsupported impossibility claim. */
int ch_check(const struct ch_op *, unsigned n, unsigned capacity, unsigned spec,
             unsigned extend_pending, unsigned event_mode, uint32_t budget,
             struct ch_witness *, uint32_t *visited);
#endif
