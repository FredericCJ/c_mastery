#include "event_follow.h"
#include <stdint.h>
/* Context points to one persistent uint32_t saved mask; non-reentrant UP only. */
static void irq_enter(void *ctx)
{uint32_t saved;__asm__ volatile("mrs %0, primask\n\tcpsid i":"=r"(saved)::"memory");*(uint32_t *)ctx=saved;}
static void irq_leave(void *ctx)
{uint32_t saved=*(uint32_t *)ctx;__asm__ volatile("dmb sy\n\tmsr primask, %0"::"r"(saved):"memory");}
struct pf_lock pf_irq_lock(uint32_t *saved){struct pf_lock lock={saved,irq_enter,irq_leave};return lock;}
