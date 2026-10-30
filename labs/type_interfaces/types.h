/*
 * Typed interfaces: arrays, function types, opaque owners, unit wrappers,
 * callbacks with a context pointer and a retained deferred job.
 *
 * Status contract shared by every function below: a function returns
 * TD_INVALID for a null required pointer, and on any failure it leaves its
 * output parameter and the caller's objects unchanged. No function blocks,
 * allocates or retains a pointer beyond what its comment says.
 */
#ifndef TD_TYPES_H
#define TD_TYPES_H
#include <stddef.h>

enum td_status
{
    TD_OK,
    /* Null or otherwise unusable argument. */
    TD_INVALID,
    /* Value outside the admitted interval, or an inexact division. */
    TD_RANGE,
    /* Operation attempted while a job is running, or td_reset on a pending
     * or running job. */
    TD_BUSY,
    /* Operation not allowed in the current job state. */
    TD_STATE,
    /* Scaffold stub: not yet implemented by the learner. */
    TD_UNFINISHED
};

typedef int td_row[4];
typedef int td_unary(int);
typedef td_unary *td_unary_ptr;

/* A function pointer inside a struct: callable only as wrapped.call(x). */
struct td_callable
{
    td_unary_ptr call;
};

/* Returns a pointer to a static row of four ints {1, 2, 3, 4}. */
int (*td_row_factory(void))[4];

/* Returns a function that doubles its argument (admitted range +-1000). */
td_unary_ptr td_factory(void);

/* Same function as td_factory, wrapped in a struct. */
struct td_callable td_wrapped_factory(void);

/*
 * Opaque counter. The type is incomplete here, so callers hold pointers only.
 * td_owner(index) returns one of two static owners (index 0 or 1) or NULL for
 * any other index; the caller never frees it. Each owner starts at 0 and holds
 * a value in [-1000, 1000].
 */
struct td_opaque;
struct td_opaque *td_owner(unsigned index);

/* Stores the owner's value in *out. TD_INVALID if owner or out is NULL. */
enum td_status td_get(const struct td_opaque *owner, int *out);

/*
 * Adds delta in [-1000, 1000]. TD_RANGE if delta or the resulting value leaves
 * [-1000, 1000]; the stored value is then unchanged.
 */
enum td_status td_add(struct td_opaque *owner, int delta);

/* Distinct single-field types so units cannot be swapped silently. */
struct td_meters
{
    int value; /* metres, 0..1000 */
};

struct td_seconds
{
    int value; /* seconds, 1..1000 */
};

struct td_speed
{
    int value; /* metres per second, 0..1000 */
};

/*
 * Exact integer speed = distance / duration. TD_RANGE if an input is outside
 * its interval, the duration is zero or the division is inexact. *out is
 * written only on TD_OK.
 */
enum td_status td_speed(struct td_meters distance, struct td_seconds duration,
                        struct td_speed *out);

/*
 * Distance = speed * duration. TD_RANGE if an input is outside its interval or
 * the product exceeds 1000 metres. *out is written only on TD_OK.
 */
enum td_status td_distance(struct td_speed speed, struct td_seconds duration,
                           struct td_meters *out);

/* Caller-owned state for the td_step callback. */
struct td_context
{
    int bias;
    unsigned calls; /* incremented once per td_step call */
};

/* Returns value + context->bias and counts the call. context must be live. */
int td_step(int value, void *context);

/* td_step with the (context, value) order that td_dispatch_fn requires. */
int td_adapter(void *context, int value);

typedef int (*td_dispatch_fn)(void *, int);

/*
 * Sums callback(context, element) over `count` rows of four ints, row by row.
 * Preconditions: count <= 4; elements in [-1000, 1000]. count == 0 yields 0
 * without calling back and accepts null rows and callback. Every element is
 * checked before the first callback, so a TD_RANGE or TD_INVALID result means
 * the callback was never called. The callback and context are borrowed for
 * the duration of the call only.
 */
enum td_status td_rows(const int (*rows)[4], size_t count,
                       td_dispatch_fn callback, void *context, int *out);

enum td_job_state
{
    TD_IDLE,
    TD_PENDING,
    TD_RUNNING,
    TD_DONE,
    TD_CANCELED
};

/*
 * A deferred call. The job borrows `callback` and `context` from the caller
 * from td_submit until the job is done, canceled or reset; the caller keeps
 * `context` alive that long. Single-threaded: TD_BUSY marks re-entry from
 * inside the callback (td_reset also returns it for a pending job), not
 * another thread.
 */
struct td_job
{
    enum td_job_state state;
    td_dispatch_fn callback;
    void *context;
    int argument;
};

#define TD_JOB_INIT {TD_IDLE, NULL, NULL, 0}

/* IDLE -> PENDING. TD_STATE if the job is not idle; TD_INVALID if null. */
enum td_status td_submit(struct td_job *job, td_dispatch_fn callback,
                         void *context, int argument);

/*
 * PENDING -> RUNNING -> DONE: calls callback(context, argument) once, stores
 * the result in *out and drops the borrowed pointers. TD_BUSY if already
 * running, TD_STATE otherwise when not pending.
 */
enum td_status td_run(struct td_job *job, int *out);

/*
 * PENDING -> CANCELED: drops the borrowed pointers so the callback is never
 * called. TD_BUSY while running, TD_STATE when not pending.
 */
enum td_status td_cancel(struct td_job *job);

/* Back to IDLE from IDLE, DONE or CANCELED. TD_BUSY if pending or running. */
enum td_status td_reset(struct td_job *job);
#endif
