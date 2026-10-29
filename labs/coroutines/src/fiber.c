#include "fiber.h"

int co_fiber_init(struct co_fiber *f, void *s, size_t n, co_fiber_fn fn,
                  void *a)
{
    (void)f;
    (void)s;
    (void)n;
    (void)fn;
    (void)a;
    return 9;
}

int co_fiber_resume(struct co_fiber *f)
{
    (void)f;
    return 9;
}

int co_fiber_yield(struct co_fiber *f)
{
    (void)f;
    return 9;
}

int co_fiber_destroy(struct co_fiber *f)
{
    (void)f;
    return 9;
}
