#include "types.h"
#ifndef TD_MUTANT
#define TD_MUTANT 0
#endif
struct td_opaque
{
    int value;
};
static struct td_opaque owners[2];
static td_row row = {1, 2, 3, 4};

static int twice(int x)
{
    return x * 2;
} /* admitted x in [-1000,1000] */

int (*td_row_factory(void))[4]
{
    return &row;
}

td_unary_ptr td_factory(void)
{
    return twice;
}

struct td_callable td_wrapped_factory(void)
{
    return (struct td_callable){twice};
}

struct td_opaque *td_owner(unsigned i)
{
    if (i >= 2)
    {
        return NULL;
    }
    return &owners[TD_MUTANT == 1 ? 0 : i];
}

enum td_status td_get(const struct td_opaque *owner, int *out)
{
    if (!owner || !out)
    {
        return TD_INVALID;
    }
    *out = owner->value;
    return TD_OK;
}

enum td_status td_add(struct td_opaque *owner, int delta)
{
    if (!owner)
    {
        return TD_INVALID;
    }
    if (delta < -1000 || delta > 1000 || owner->value + delta < -1000 ||
        owner->value + delta > 1000)
    {
        return TD_RANGE;
    }
    owner->value += delta;
    return TD_OK;
}

enum td_status td_speed(struct td_meters d, struct td_seconds t,
                        struct td_speed *out)
{
    if (!out)
    {
        return TD_INVALID;
    }
    if (d.value < 0 || d.value > 1000 || t.value < 1 || t.value > 1000)
    {
        return TD_RANGE;
    }
    if (TD_MUTANT != 2 && d.value % t.value)
    {
        return TD_RANGE;
    }
    *out = (struct td_speed){d.value / t.value};
    return TD_OK;
}

enum td_status td_distance(struct td_speed v, struct td_seconds t,
                           struct td_meters *out)
{
    if (!out)
    {
        return TD_INVALID;
    }
    if (v.value < 0 || v.value > 1000 || t.value < 1 || t.value > 1000 ||
        v.value > 1000 / t.value)
    {
        return TD_RANGE;
    }
    *out = (struct td_meters){v.value * t.value};
    return TD_OK;
}

int td_step(int value, void *context)
{
    struct td_context *c =
        context; /* contract: live context, bounded value/bias/calls */
    ++c->calls;
    return value + c->bias;
}

int td_adapter(void *context, int value)
{
    return td_step(value, context);
}

enum td_status td_rows(const int (*rows)[4], size_t n, td_dispatch_fn callback,
                       void *context, int *out)
{
    if (!out || (n && (!rows || !callback)))
    {
        return TD_INVALID;
    }
    if (n > 4)
    {
        return TD_RANGE;
    }
    int sum = 0;
    /* Validate every row element before any callback or result mutation. */
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            if (rows[i][j] < -1000 || rows[i][j] > 1000)
            {
                return TD_RANGE;
            }
        }
    }
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < 4; ++j)
        {
            sum += callback(context, rows[TD_MUTANT == 3 ? 0 : i][j]);
        }
    }
    *out = sum;
    return TD_OK;
}

enum td_status td_submit(struct td_job *j, td_dispatch_fn cb, void *ctx,
                         int arg)
{
    if (!j || !cb)
    {
        return TD_INVALID;
    }
    if (j->state != TD_IDLE)
    {
        return TD_STATE;
    }
    j->callback = cb;
    j->context = ctx;
    j->argument = arg;
    j->state = TD_PENDING;
    return TD_OK;
}

enum td_status td_run(struct td_job *j, int *out)
{
    if (!j || !out)
    {
        return TD_INVALID;
    }
    if (j->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    if (j->state != TD_PENDING)
    {
        return TD_STATE;
    }
    j->state = TD_RUNNING;
    int value = j->callback(j->context, j->argument);
    *out = value;
    j->callback = NULL;
    j->context = NULL;
    j->state = TD_DONE;
    return TD_OK;
}

enum td_status td_cancel(struct td_job *j)
{
    if (!j)
    {
        return TD_INVALID;
    }
    if (j->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    if (j->state != TD_PENDING)
    {
        return TD_STATE;
    }
    if (TD_MUTANT != 4)
    {
        j->callback = NULL;
        j->context = NULL;
        j->state = TD_CANCELED;
    }
    return TD_OK;
}

enum td_status td_reset(struct td_job *j)
{
    if (!j)
    {
        return TD_INVALID;
    }
    if (j->state == TD_PENDING || j->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    *j = (struct td_job)TD_JOB_INIT;
    return TD_OK;
}
