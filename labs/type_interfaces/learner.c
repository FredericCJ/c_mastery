/*
 * Learner scaffold: every function is an unfinished stub that returns its
 * sentinel (NULL, 0 or TD_UNFINISHED). Replace each body with the behaviour
 * documented in types.h and CONTRACTS.md; the tests reject the stubs.
 */
#include "types.h"

int (*td_row_factory(void))[4]
{
    return NULL;
}

td_unary_ptr td_factory(void)
{
    return NULL;
}

struct td_callable td_wrapped_factory(void)
{
    return (struct td_callable){NULL};
}

struct td_opaque *td_owner(unsigned index)
{
    (void)index;
    return NULL;
}

enum td_status td_get(const struct td_opaque *owner, int *out)
{
    (void)owner;
    (void)out;
    return TD_UNFINISHED;
}

enum td_status td_add(struct td_opaque *owner, int delta)
{
    (void)owner;
    (void)delta;
    return TD_UNFINISHED;
}

enum td_status td_speed(struct td_meters distance, struct td_seconds duration,
                        struct td_speed *out)
{
    (void)distance;
    (void)duration;
    (void)out;
    return TD_UNFINISHED;
}

enum td_status td_distance(struct td_speed speed, struct td_seconds duration,
                           struct td_meters *out)
{
    (void)speed;
    (void)duration;
    (void)out;
    return TD_UNFINISHED;
}

int td_step(int value, void *context)
{
    (void)value;
    (void)context;
    return 0;
}

int td_adapter(void *context, int value)
{
    (void)context;
    (void)value;
    return 0;
}

enum td_status td_rows(const int (*rows)[4], size_t count,
                       td_dispatch_fn callback, void *context, int *out)
{
    (void)rows;
    (void)count;
    (void)callback;
    (void)context;
    (void)out;
    return TD_UNFINISHED;
}

enum td_status td_submit(struct td_job *job, td_dispatch_fn callback,
                         void *context, int argument)
{
    (void)job;
    (void)callback;
    (void)context;
    (void)argument;
    return TD_UNFINISHED;
}

enum td_status td_run(struct td_job *job, int *out)
{
    (void)job;
    (void)out;
    return TD_UNFINISHED;
}

enum td_status td_cancel(struct td_job *job)
{
    (void)job;
    return TD_UNFINISHED;
}

enum td_status td_reset(struct td_job *job)
{
    (void)job;
    return TD_UNFINISHED;
}
