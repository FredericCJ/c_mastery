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

struct td_opaque *td_owner(unsigned i)
{
    (void)i;
    return NULL;
}

enum td_status td_get(const struct td_opaque *p, int *o)
{
    (void)p;
    (void)o;
    return TD_UNFINISHED;
}

enum td_status td_add(struct td_opaque *p, int v)
{
    (void)p;
    (void)v;
    return TD_UNFINISHED;
}

enum td_status td_speed(struct td_meters d, struct td_seconds t,
                        struct td_speed *o)
{
    (void)d;
    (void)t;
    (void)o;
    return TD_UNFINISHED;
}

enum td_status td_distance(struct td_speed v, struct td_seconds t,
                           struct td_meters *o)
{
    (void)v;
    (void)t;
    (void)o;
    return TD_UNFINISHED;
}

int td_step(int v, void *c)
{
    (void)v;
    (void)c;
    return 0;
}

int td_adapter(void *c, int v)
{
    (void)v;
    (void)c;
    return 0;
}

enum td_status td_rows(const int (*a)[4], size_t n, td_dispatch_fn f, void *c,
                       int *o)
{
    (void)a;
    (void)n;
    (void)f;
    (void)c;
    (void)o;
    return TD_UNFINISHED;
}

enum td_status td_submit(struct td_job *j, td_dispatch_fn f, void *c, int v)
{
    (void)j;
    (void)f;
    (void)c;
    (void)v;
    return TD_UNFINISHED;
}

enum td_status td_run(struct td_job *j, int *o)
{
    (void)j;
    (void)o;
    return TD_UNFINISHED;
}

enum td_status td_cancel(struct td_job *j)
{
    (void)j;
    return TD_UNFINISHED;
}

enum td_status td_reset(struct td_job *j)
{
    (void)j;
    return TD_UNFINISHED;
}
