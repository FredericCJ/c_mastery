#include "cmx_port.h"
/* Intentionally unfinished. Preserve the machine contract when replacing. */
cmx_kernel cmx_machine;
volatile uint32_t cmx_port_ticks, cmx_port_switches, cmx_port_status;

uint32_t cmx_port_lock(void)
{
    return 0;
}

void cmx_port_unlock(uint32_t saved)
{
    (void)saved;
}

void cmx_port_init(void)
{
    cmx_init(&cmx_machine, CMX_RR);
}

int cmx_port_task(void (*entry)(void), uint32_t *stack, unsigned words,
                  unsigned priority)
{
    (void)entry;
    (void)stack;
    (void)words;
    (void)priority;
    return CMX_NONE;
}

uint32_t *cmx_port_choose(uint32_t *old)
{
    (void)old;
    return 0;
}

void cmx_port_yield(void)
{
}

void cmx_port_start(void)
{
    cmx_port_finish(1);
}

void SysTick_Handler(void)
{
}

enum cmx_result cmx_port_take(cmx_sem *s, cmx_deadline d)
{
    (void)s;
    (void)d;
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_port_give(cmx_sem *s)
{
    (void)s;
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_port_send(cmx_queue *q, uint32_t v, cmx_deadline d)
{
    (void)q;
    (void)v;
    (void)d;
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_port_recv(cmx_queue *q, uint32_t *v, cmx_deadline d)
{
    (void)q;
    (void)v;
    (void)d;
    return CMX_UNIMPLEMENTED;
}

void cmx_port_close(cmx_queue *q)
{
    (void)q;
}

void cmx_port_finish(unsigned code)
{
    cmx_port_status = 0xdead0000u | code;
    for (;;)
        __asm__ volatile("wfi");
}

void cmx_fault(void)
{
    cmx_port_finish(2);
}
