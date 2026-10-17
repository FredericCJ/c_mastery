#ifndef CMASTERY_FIBER_H
#define CMASTERY_FIBER_H
#include <stddef.h>
#include <stdint.h>
/* Linux x86-64 System V ABI only. This is an ABI contract, not ISO C. */
typedef struct {
 uintptr_t rsp,rbx,rbp,r12,r13,r14,r15;
 uint32_t mxcsr;uint16_t x87_control,reserved;
 void *fake_stack;const void *bottom;size_t size;void *tsan;
} COContext;
typedef struct COFiber COFiber;
typedef void (*COFiberFn)(COFiber*,void*);
struct COFiber {
 COContext context;COContext *caller;COFiberFn run;void *argument;
 unsigned state; /* 0 fresh,1 suspended,2 running,3 completed,4 destroyed */
};
int co_fiber_init(COFiber*,void *stack,size_t,COFiberFn,void*);
int co_fiber_resume(COFiber*);
int co_fiber_yield(COFiber*);
int co_fiber_destroy(COFiber*);
#endif
