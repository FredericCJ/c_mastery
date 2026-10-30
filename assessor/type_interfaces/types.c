/*
 * Reference implementation of the type-interface lab (labs/type_interfaces).
 *
 * TD_MUTANT selects one deliberate defect for the mutation campaign; 0 is the
 * correct behaviour. Each hook is marked "mutant N" where it is applied.
 */
#include "types.h"

#ifndef TD_MUTANT
#define TD_MUTANT 0
#endif

enum
{
    VALUE_MIN = -1000, /* admitted value, inclusive */
    VALUE_MAX = 1000, /* admitted value, inclusive */
    OWNER_COUNT = 2,
    ROW_WIDTH = 4,
    MAX_ROWS = 4
};

struct td_opaque
{
    int value; /* invariant: VALUE_MIN <= value <= VALUE_MAX */
};

static struct td_opaque owners[OWNER_COUNT];
static td_row row = {1, 2, 3, 4};

static int in_value_range(int value)
{
    return value >= VALUE_MIN && value <= VALUE_MAX;
}

/* Exact for every x admitted by in_value_range: |2x| fits in int. */
static int twice(int x)
{
    return x * 2;
}

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

struct td_opaque *td_owner(unsigned index)
{
    if (index >= OWNER_COUNT)
    {
        return NULL;
    }

    /* mutant 1: every index aliases owner 0. */
    unsigned slot = (TD_MUTANT == 1) ? 0 : index;
    return &owners[slot];
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

    /* Both operands are in [-1000, 1000], so the sum cannot overflow. */
    if (!in_value_range(delta) || !in_value_range(owner->value + delta))
    {
        return TD_RANGE;
    }
    owner->value += delta;
    return TD_OK;
}

enum td_status td_speed(struct td_meters distance, struct td_seconds duration,
                        struct td_speed *out)
{
    if (!out)
    {
        return TD_INVALID;
    }
    if (distance.value < 0 || distance.value > VALUE_MAX ||
        duration.value < 1 || duration.value > VALUE_MAX)
    {
        return TD_RANGE;
    }

    /* mutant 2: accept inexact division and truncate. */
    int inexact = distance.value % duration.value != 0;
    if (TD_MUTANT != 2 && inexact)
    {
        return TD_RANGE;
    }
    *out = (struct td_speed){distance.value / duration.value};
    return TD_OK;
}

enum td_status td_distance(struct td_speed speed, struct td_seconds duration,
                           struct td_meters *out)
{
    if (!out)
    {
        return TD_INVALID;
    }
    if (speed.value < 0 || speed.value > VALUE_MAX || duration.value < 1 ||
        duration.value > VALUE_MAX)
    {
        return TD_RANGE;
    }

    /* speed * duration <= 1000 without forming the product (no overflow). */
    if (speed.value > VALUE_MAX / duration.value)
    {
        return TD_RANGE;
    }
    *out = (struct td_meters){speed.value * duration.value};
    return TD_OK;
}

/*
 * Contract: context points to a live struct td_context; value and bias are
 * small enough that the sum does not overflow.
 */
int td_step(int value, void *context)
{
    struct td_context *state = context;

    ++state->calls;
    return value + state->bias;
}

int td_adapter(void *context, int value)
{
    return td_step(value, context);
}

/* True when every element of the first `count` rows is in the value range. */
static int rows_in_range(const int (*rows)[ROW_WIDTH], size_t count)
{
    for (size_t r = 0; r < count; ++r)
    {
        for (size_t column = 0; column < ROW_WIDTH; ++column)
        {
            if (!in_value_range(rows[r][column]))
            {
                return 0;
            }
        }
    }
    return 1;
}

enum td_status td_rows(const int (*rows)[4], size_t count,
                       td_dispatch_fn callback, void *context, int *out)
{
    if (!out || (count && (!rows || !callback)))
    {
        return TD_INVALID;
    }
    if (count > MAX_ROWS)
    {
        return TD_RANGE;
    }

    /* Admit the whole batch before the first callback or result write. */
    if (!rows_in_range(rows, count))
    {
        return TD_RANGE;
    }

    int sum = 0;
    for (size_t r = 0; r < count; ++r)
    {
        /* mutant 3: every row reads row 0 (wrong stride). */
        size_t source_row = (TD_MUTANT == 3) ? 0 : r;
        for (size_t column = 0; column < ROW_WIDTH; ++column)
        {
            sum += callback(context, rows[source_row][column]);
        }
    }
    *out = sum;
    return TD_OK;
}

enum td_status td_submit(struct td_job *job, td_dispatch_fn callback,
                         void *context, int argument)
{
    if (!job || !callback)
    {
        return TD_INVALID;
    }
    if (job->state != TD_IDLE)
    {
        return TD_STATE;
    }
    job->callback = callback;
    job->context = context;
    job->argument = argument;
    job->state = TD_PENDING;
    return TD_OK;
}

enum td_status td_run(struct td_job *job, int *out)
{
    if (!job || !out)
    {
        return TD_INVALID;
    }
    if (job->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    if (job->state != TD_PENDING)
    {
        return TD_STATE;
    }

    /*
     * RUNNING is set before the callback so a re-entrant td_run, td_cancel or
     * td_reset sees TD_BUSY instead of corrupting the job.
     */
    job->state = TD_RUNNING;
    int value = job->callback(job->context, job->argument);
    *out = value;

    /* Release the borrowed pointers: the job is quiescent from here on. */
    job->callback = NULL;
    job->context = NULL;
    job->state = TD_DONE;
    return TD_OK;
}

enum td_status td_cancel(struct td_job *job)
{
    if (!job)
    {
        return TD_INVALID;
    }
    if (job->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    if (job->state != TD_PENDING)
    {
        return TD_STATE;
    }

    /* mutant 4: report success but leave the job pending. */
    if (TD_MUTANT != 4)
    {
        job->callback = NULL;
        job->context = NULL;
        job->state = TD_CANCELED;
    }
    return TD_OK;
}

enum td_status td_reset(struct td_job *job)
{
    if (!job)
    {
        return TD_INVALID;
    }
    if (job->state == TD_PENDING || job->state == TD_RUNNING)
    {
        return TD_BUSY;
    }
    *job = (struct td_job)TD_JOB_INIT;
    return TD_OK;
}
