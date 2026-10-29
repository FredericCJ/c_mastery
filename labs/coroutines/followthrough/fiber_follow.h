#ifndef CMASTERY_FIBER_FOLLOW_H
#define CMASTERY_FIBER_FOLLOW_H
#include "fiber.h"
struct cf_fiber;
typedef void (*cf_fiber_fn)(struct cf_fiber *, void *);

struct cf_fiber
{
    struct co_fiber fiber;
    void *mapping;
    size_t mapping_size, page_size;
    unsigned cancel;
    cf_fiber_fn run;
    void *argument;
};

int cf_fiber_create(struct cf_fiber *, size_t, cf_fiber_fn, void *);
int cf_fiber_resume(struct cf_fiber *);
int cf_fiber_suspend(struct cf_fiber *);
int cf_fiber_cancel(struct cf_fiber *);
int cf_fiber_destroy(struct cf_fiber *);
#endif
