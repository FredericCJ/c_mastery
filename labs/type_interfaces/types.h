#ifndef TD_TYPES_H
#define TD_TYPES_H
#include <stddef.h>

enum td_status
{
    TD_OK,
    TD_INVALID,
    TD_RANGE,
    TD_BUSY,
    TD_STATE,
    TD_UNFINISHED
};

typedef int td_row[4];
typedef int td_unary(int);
typedef td_unary *td_unary_ptr;

struct td_callable
{
    td_unary_ptr call;
};

int (*td_row_factory(void))[4];
td_unary_ptr td_factory(void);
struct td_callable td_wrapped_factory(void);
struct td_opaque;
struct td_opaque *td_owner(unsigned index);
enum td_status td_get(const struct td_opaque *owner, int *out);
enum td_status td_add(struct td_opaque *owner, int delta);

struct td_meters
{
    int value;
};

struct td_seconds
{
    int value;
};

struct td_speed
{
    int value;
};
enum td_status td_speed(struct td_meters distance, struct td_seconds duration,
                        struct td_speed *out);
enum td_status td_distance(struct td_speed speed, struct td_seconds duration,
                           struct td_meters *out);

struct td_context
{
    int bias;
    unsigned calls;
};

int td_step(int value, void *context);
int td_adapter(void *context, int value);
typedef int (*td_dispatch_fn)(void *, int);
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

struct td_job
{
    enum td_job_state state;
    td_dispatch_fn callback;
    void *context;
    int argument;
};

#define TD_JOB_INIT {TD_IDLE, NULL, NULL, 0}
enum td_status td_submit(struct td_job *job, td_dispatch_fn callback,
                         void *context, int argument);
enum td_status td_run(struct td_job *job, int *out);
enum td_status td_cancel(struct td_job *job);
enum td_status td_reset(struct td_job *job);
#endif
