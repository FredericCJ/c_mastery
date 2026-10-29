#ifndef CMASTERY_FIBER_H
#define CMASTERY_FIBER_H
#include <stddef.h>
#include <stdint.h>

/* Linux x86-64 System V ABI only. This is an ABI contract, not ISO C. */
struct co_context
{
    uintptr_t rsp, rbx, rbp, r12, r13, r14, r15;
    uint32_t mxcsr;
    uint16_t x87_control, reserved;
    void *fake_stack;
    const void *bottom;
    size_t size;
    void *tsan;
};
struct co_fiber;
typedef void (*co_fiber_fn)(struct co_fiber *, void *);

struct co_fiber
{
    struct co_context context;
    struct co_context *caller;
    co_fiber_fn run;
    void *argument;
    unsigned state; /* 0 fresh,1 suspended,2 running,3 completed,4 destroyed */
};

int co_fiber_init(struct co_fiber *, void *stack, size_t, co_fiber_fn, void *);
int co_fiber_resume(struct co_fiber *);
int co_fiber_yield(struct co_fiber *);
int co_fiber_destroy(struct co_fiber *);
#endif
