#include "lifecycle.h"

enum bu_result bu_drain_begin(struct bu_drain *d, unsigned n, unsigned limit)
{
    (void)d;
    (void)n;
    (void)limit;
    return BU_UNSUPPORTED;
}

enum bu_result bu_drain_step(struct bu_drain *d, unsigned ack, unsigned done)
{
    (void)d;
    (void)ack;
    (void)done;
    return BU_UNSUPPORTED;
}

enum bu_result bu_task_start(struct bu_task *t)
{
    (void)t;
    return BU_UNSUPPORTED;
}

enum bu_result bu_task_return(struct bu_task *t, unsigned g, int result,
                              unsigned n)
{
    (void)t;
    (void)g;
    (void)result;
    (void)n;
    return BU_UNSUPPORTED;
}

enum bu_result bu_task_drained(struct bu_task *t, unsigned g)
{
    (void)t;
    (void)g;
    return BU_UNSUPPORTED;
}
