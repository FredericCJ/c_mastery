#ifndef CMASTERY_ABSTRACTION_REDUCER_H
#define CMASTERY_ABSTRACTION_REDUCER_H
#include "session.h"

enum am_state
{
    AM_IDLE,
    AM_READY,
    AM_WAIT,
    AM_FAILED
};

enum am_event
{
    AM_START,
    AM_SEND,
    AM_ACK,
    AM_TIMEOUT,
    AM_EFFECT_FAILURE,
    AM_CANCEL
};

struct am_machine
{
    enum am_state state;
    unsigned sequence;
};

enum am_effect_kind
{
    AM_NO_EFFECT,
    AM_WRITE
};

struct am_effect
{
    enum am_effect_kind kind;
    unsigned key;
};

struct am_transition
{
    struct am_machine next;
    struct am_effect effect;
};
enum as_status am_switch(struct am_machine, enum am_event,
                         struct am_transition *);
enum as_status am_table(struct am_machine, enum am_event,
                        struct am_transition *);
enum as_status am_functions(struct am_machine, enum am_event,
                            struct am_transition *);
enum as_status am_sparse(unsigned sparse_state, unsigned sequence,
                         enum am_event, struct am_transition *);

struct am_backend
{
    unsigned committed_key, value, attempts, fail_before, lose_ack;
};
enum as_status am_execute(struct am_backend *, struct am_effect);
typedef enum as_status (*am_execute_fn)(void *, struct am_effect);
typedef void (*am_release_fn)(void *);

struct am_strategy
{
    am_execute_fn execute;
    am_release_fn release;
    void *context;
    unsigned epoch;
};

struct am_hot
{
    struct am_strategy current, retired;
    unsigned inflight_current, inflight_retired;
};

struct am_lease
{
    struct am_strategy selected;
    unsigned live;
};
enum as_status am_hot_acquire(struct am_hot *, struct am_lease *);
enum as_status am_hot_replace(struct am_hot *, struct am_strategy);
enum as_status am_hot_release(struct am_hot *, struct am_lease *);
enum as_status am_hot_close(struct am_hot *);
#endif
