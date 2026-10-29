#include "fiber_follow.h"

int cf_fiber_create(struct cf_fiber *a, size_t b, cf_fiber_fn c, void *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 9;
}

int cf_fiber_resume(struct cf_fiber *a)
{
    (void)a;
    return 9;
}

int cf_fiber_suspend(struct cf_fiber *a)
{
    (void)a;
    return 9;
}

int cf_fiber_cancel(struct cf_fiber *a)
{
    (void)a;
    return 9;
}

int cf_fiber_destroy(struct cf_fiber *a)
{
    (void)a;
    return 9;
}
