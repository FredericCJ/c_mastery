#include "fiber.h"
#include <stddef.h>
_Static_assert(offsetof(struct co_context, mxcsr) == 56, "assembly ABI offset");
_Static_assert(offsetof(struct co_context, x87_control) == 60,
               "assembly ABI offset");
#if defined(CO_ASAN)
#include <sanitizer/common_interface_defs.h>
#endif
#if defined(CO_TSAN)
#include <sanitizer/tsan_interface.h>
#endif
extern void co_context_switch(struct co_context *, const struct co_context *);
extern void co_context_entry(void);

/* TSan changes the current logical stack immediately. An out-of-line helper
 * epilogue would issue its function-exit event on the destination context. */
static inline __attribute__((always_inline)) void
start(struct co_context *from, struct co_context *to, unsigned terminal)
{
#if defined(CO_ASAN)
    __sanitizer_start_switch_fiber(terminal ? NULL : &from->fake_stack,
                                   to->bottom, to->size);
#else
    (void)terminal;
#endif
#if defined(CO_TSAN)
    __tsan_switch_to_fiber(to->tsan, 0);
#endif
    (void)from;
    (void)to;
}

static void finish(struct co_context *current, struct co_context *previous)
{
#if defined(CO_ASAN)
    __sanitizer_finish_switch_fiber(current->fake_stack, &previous->bottom,
                                    &previous->size);
#else
    (void)current;
    (void)previous;
#endif
}

int co_fiber_init(struct co_fiber *f, void *stack, size_t n, co_fiber_fn fn,
                  void *arg)
{
    if (!f || !stack || !fn || n < 65536 || (uintptr_t)stack % 16 || n % 16)
    {
        return 1;
    }
    /* The caller provides one live disjoint stack object and an uninitialized
     * fiber. Reinitializing a live fiber is outside the initialization contract. */
    struct co_fiber fresh = {0};
    fresh.context.bottom = stack;
    fresh.context.size = n;
    fresh.run = fn;
    fresh.argument = arg;
    uintptr_t *sp = (uintptr_t *)((unsigned char *)stack + n) - 2;
    sp[0] = (uintptr_t)co_context_entry;
    sp[1] = 0;
    fresh.context.rsp = (uintptr_t)sp;
    fresh.context.r12 = (uintptr_t)f;
    __asm__ volatile("stmxcsr %0" : "=m"(fresh.context.mxcsr));
    __asm__ volatile("fnstcw %0" : "=m"(fresh.context.x87_control));
#if defined(CO_TSAN)
    fresh.context.tsan = __tsan_create_fiber(0);
    if (!fresh.context.tsan)
    {
        return 2;
    }
#endif
    *f = fresh;
    return 0;
}

void co_fiber_boot(struct co_fiber *f)
{
    finish(&f->context, f->caller);
    f->run(f, f->argument);
    f->state = 3;
    start(&f->context, f->caller, 1);
    co_context_switch(&f->context, f->caller);
    __builtin_trap();
}

int co_fiber_resume(struct co_fiber *f)
{
    if (!f)
    {
        return 1;
    }
    if (f->state > 1)
    {
        return 3;
    }
    struct co_context caller = {0};
#if defined(CO_TSAN)
    caller.tsan = __tsan_get_current_fiber();
#endif
    f->caller = &caller;
    f->state = 2;
    start(&caller, &f->context, 0);
    co_context_switch(&caller, &f->context);
    finish(&caller, &f->context);
    f->caller = NULL;
    return f->state == 3 ? 7 : 4;
}

int co_fiber_yield(struct co_fiber *f)
{
    if (!f || f->state != 2 || !f->caller)
    {
        return 3;
    }
    f->state = 1;
    start(&f->context, f->caller, 0);
    co_context_switch(&f->context, f->caller);
    finish(&f->context, f->caller);
    return 0;
}

int co_fiber_destroy(struct co_fiber *f)
{
    if (!f)
    {
        return 1;
    }
#ifndef CR_MUTATE_FIBER2
    if (f->state != 0 && f->state != 3)
    {
        return 3;
    }
#endif
#if defined(CO_TSAN)
    __tsan_destroy_fiber(f->context.tsan);
    f->context.tsan = NULL;
#endif
    f->state = 4;
    return 0;
}
