#include "cbs.h"
static struct cb_trace trace;
static struct cb_mutex_trace mutex_trace;

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const void *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

static void finish(unsigned code)
{
    unsigned args[2] = {0x20026u, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register const void *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    for (;;)
    {
    }
}

#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            say("CB_M4_CHECK\n"); \
            finish(1);            \
        }                         \
    } while (0)

void task_main(void)
{
    struct cb_budget s, a, b;
    CHECK(cb_init(&s, 3, 10, 10, CB_SOFT_BOOK, 0) == CMX_OK &&
          cb_arrive(&s, 0) == CMX_OK);
    for (unsigned t = 0; t < 8; t++)
        CHECK(cb_charge(&s, t, 1) == CMX_OK && !s.throttled);
    CHECK(s.core.remaining == 1 && s.core.deadline == 30);
    CHECK(cb_init(&a, 4, 8, 8, CB_SOFT_BOOK, 0) == CMX_OK &&
          cb_init(&b, 4, 8, 8, CB_SOFT_RETAINED, 0) == CMX_OK);
    CHECK(cb_arrive(&a, 0) == CMX_OK && cb_arrive(&b, 0) == CMX_OK &&
          cb_charge(&a, 0, 2) == CMX_OK && cb_charge(&b, 0, 2) == CMX_OK);
    CHECK(cb_idle(&a) == CMX_OK && cb_idle(&b) == CMX_OK &&
          cb_arrive(&a, 4) == CMX_OK && cb_arrive(&b, 4) == CMX_OK);
    CHECK(a.core.deadline == 12 && a.core.remaining == 4 &&
          b.core.deadline == 8 && b.core.remaining == 2);
    CHECK(cb_init(&s, 2, 4, 10, CB_HARD_DOCUMENT, 0) == CMX_OK &&
          cb_arrive(&s, 0) == CMX_OK && s.core.deadline == 4);
    CHECK(cb_charge(&s, 0, 2) == CMX_OK && s.throttled &&
          cb_update(&s, 4) == CMX_OK && s.core.deadline == 14);
    unsigned admitted = 99;
    CHECK(cb_admit(1073741824, 2147483647, 2147483647, 1073741824, 2147483647,
                   2147483647, &admitted) == CMX_OK &&
          !admitted);
    struct cb_workload w = {.quota = {3, 2},
                            .deadline = {10, 5},
                            .period = {10, 5},
                            .policy = {CB_SOFT_BOOK, CB_SOFT_BOOK},
                            .count = 1,
                            .horizon = 32,
                            .capacity = 8};
    w.request[0] = (struct cb_request){0, 0, 8, 10};
    CHECK(cb_workload(&w, &trace) == CMX_OK && trace.finish[0] == 8);
    w.policy[0] = CB_HARD_DOCUMENT;
    CHECK(cb_workload(&w, &trace) == CMX_OK && trace.finish[0] == 22);
    CHECK(cb_mutex(7, &mutex_trace) == CMX_OK && mutex_trace.waiter_missed &&
          mutex_trace.waiter_finish == 8);
    for (unsigned t = 1; t < 7; t++)
        CHECK(mutex_trace.charged[t] == 0 && mutex_trace.budget[t][1] == 2);
    say("CB_M4_PASS:soft-throttle-equality-constrained-and-real-mutex-producer\n");
    finish(0);
}

void fault(void)
{
    say("CB_M4_FAULT\n");
    finish(2);
}

extern unsigned cl_stack_top, cl_data_load, cl_data_start, cl_data_end,
    cl_bss_start, cl_bss_end;

void cl_reset(void)
{
    unsigned *d = &cl_data_start, *s = &cl_data_load;
    while (d < &cl_data_end)
        *d++ = *s++;
    for (d = &cl_bss_start; d < &cl_bss_end;)
        *d++ = 0;
    task_main();
}

__attribute__((section(".vectors"), used)) void (*const vectors[])(void) = {
    (void (*)(void))&cl_stack_top, cl_reset, fault, fault, fault, fault, fault};
