#include "cmx_port.h"
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
cmx_kernel cmx_machine;
volatile uint32_t cmx_port_ticks, cmx_port_switches, cmx_port_status;
static uint32_t *saved_sp[CMX_TASKS];
static unsigned started;

static unsigned thread_may_wait(void)
{
    uint32_t ipsr, primask, basepri, faultmask;
    __asm__ volatile(
        "mrs %0,ipsr\nmrs %1,primask\nmrs %2,basepri\nmrs %3,faultmask"
        : "=r"(ipsr), "=r"(primask), "=r"(basepri), "=r"(faultmask));
    return started && !ipsr && !primask && !basepri && !faultmask &&
           cmx_machine.current >= 0 &&
           cmx_machine.task[cmx_machine.current].state == CMX_RUNNING;
}

__attribute__((weak)) void cmx_port_tick_hook(void)
{
}

uint32_t cmx_port_lock(void)
{
    uint32_t p;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(p)::"memory");
    return p;
}

void cmx_port_unlock(uint32_t p)
{
    __asm__ volatile("msr primask,%0\ndsb\nisb" ::"r"(p) : "memory");
}

static void pend(void)
{
    REG(0xe000ed04) = 1u << 28;
    __asm__ volatile("dsb\nisb" ::: "memory");
}

void cmx_port_yield(void)
{
    pend();
}

static void task_return(void)
{
    uint32_t p = cmx_port_lock();
    unsigned t = (unsigned)cmx_machine.current;
    if (cmx_exit(&cmx_machine, t, 0) != CMX_OK)
    {
        cmx_port_finish(21);
    }
    pend();
    cmx_port_unlock(p);
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void cmx_port_init(void)
{
    started = 0;
    cmx_init(&cmx_machine, CMX_RR);
    cmx_port_ticks = 0;
    cmx_port_switches = 0;
    cmx_port_status = 0;
    REG(0xe000ed88) |= 0xfu << 20;
    __asm__ volatile(
        "dsb\nisb" ::
            : "memory"); /* PendSV lowest; SysTick above it, below fault/SVC. */
    REG(0xe000ed20) = (REG(0xe000ed20) & 0x0000ffffu) | 0x80ff0000u;
    REG(0xe000ed14) |= 1u << 9;
    cmx_board_init();
}

int cmx_port_task(void (*entry)(void), uint32_t *stack, unsigned words,
                  unsigned priority)
{
    if (started || !entry || words < 128 || words > UINT32_MAX / sizeof *stack)
    {
        return CMX_NONE;
    }
    int id = cmx_task_add(&cmx_machine, priority, 0, (uintptr_t)stack,
                          words * sizeof *stack);
    if (id < 0)
    {
        return id;
    }
    for (unsigned i = 0; i < words; i++)
    {
        stack[i] = 0xa5a5a5a5u;
    }
    uint32_t *p = stack + words;
    *--p = 0x01000000u;
    *--p = (uint32_t)(uintptr_t)entry;
    *--p = (uint32_t)(uintptr_t)task_return;
    *--p = 0;
    *--p = 0;
    *--p = 0;
    *--p = 0;
    *--p = 0;
    *--p = 0xfffffffdu;
    for (unsigned i = 0; i < 8; i++)
    {
        *--p = 0;
    }
    saved_sp[id] = p;
    return id;
}

uint32_t *cmx_port_choose(uint32_t *old)
{
    if (old && cmx_machine.current >= 0)
    {
        unsigned t = (unsigned)cmx_machine.current;
        uintptr_t p = (uintptr_t)old;
        if (p < cmx_machine.task[t].stack_low + 32 ||
            p >= cmx_machine.task[t].stack_high)
        {
            cmx_port_finish(22);
        }
        if (*(uint32_t *)cmx_machine.task[t].stack_low != 0xa5a5a5a5u)
        {
            cmx_port_finish(23);
        }
        saved_sp[t] = old;
    }
    int next = cmx_schedule(&cmx_machine);
    if (next < 0)
    {
        cmx_port_finish(24);
    }
    cmx_port_switches++;
    return saved_sp[next];
}

void cmx_port_start(void)
{
    started = 1;
#if CMX_STAGE >= 2
    REG(0xe000e014) = 9999;
    REG(0xe000e018) = 0;
    REG(0xe000e010) = 7;
#endif
    __asm__ volatile("cpsie i\nsvc 0" ::: "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void SysTick_Handler(void)
{
    cmx_port_tick_hook();
    cmx_port_ticks++;
    cmx_tick(&cmx_machine, cmx_port_ticks);
    if (cmx_machine.current >= 0)
    {
        pend();
    }
}

enum cmx_result cmx_port_take(cmx_sem *s, cmx_deadline d)
{
    if (!thread_may_wait())
    {
        return CMX_INVALID;
    }
    uint32_t p = cmx_port_lock();
    unsigned t = (unsigned)cmx_machine.current;
    enum cmx_result r = cmx_sem_take(&cmx_machine, s, t, d);
    if (r == CMX_BLOCKED)
    {
        pend();
    }
    cmx_port_unlock(p);
    return r == CMX_BLOCKED ? cmx_machine.task[t].result : r;
}

enum cmx_result cmx_port_give(cmx_sem *s)
{
    uint32_t p = cmx_port_lock();
    enum cmx_result r = cmx_sem_give(&cmx_machine, s);
    cmx_port_unlock(p);
    return r;
}

enum cmx_result cmx_port_send(cmx_queue *q, uint32_t value, cmx_deadline d)
{
    if (!thread_may_wait())
    {
        return CMX_INVALID;
    }
    uint32_t p = cmx_port_lock();
    unsigned t = (unsigned)cmx_machine.current;
    enum cmx_result r = cmx_queue_send(&cmx_machine, q, t, value, d);
    if (r == CMX_BLOCKED)
    {
        pend();
    }
    cmx_port_unlock(p);
    return r == CMX_BLOCKED ? cmx_machine.task[t].result : r;
}

enum cmx_result cmx_port_recv(cmx_queue *q, uint32_t *value, cmx_deadline d)
{
    if (!thread_may_wait() || !value)
    {
        return CMX_INVALID;
    }
    uint32_t p = cmx_port_lock();
    unsigned t = (unsigned)cmx_machine.current;
    enum cmx_result r = cmx_queue_recv(&cmx_machine, q, t, d);
    if (r == CMX_BLOCKED)
    {
        pend();
    }
    cmx_port_unlock(p);
    if (r == CMX_BLOCKED)
    {
        r = cmx_machine.task[t].result;
    }
    if (r == CMX_OK)
    {
        *value = cmx_machine.task[t].value;
    }
    return r;
}

void cmx_port_close(cmx_queue *q)
{
    uint32_t p = cmx_port_lock();
    cmx_queue_close(&cmx_machine, q);
    cmx_port_unlock(p);
}

void cmx_port_finish(unsigned code)
{
    __asm__ volatile("cpsid i" ::: "memory");
    REG(0xe000e010) = 0;
    cmx_port_status = code ? 0xdead0000u | code : 0x600d0000u;
#ifdef CMX_QEMU
    const char *text = code ? "FAIL CMX Cortex-M image\n"
                            : "PASS CMX Cortex-M " CMX_STAGE_NAME "\n";
    register unsigned r0 __asm__("r0") = 4;
    register const void *r1 __asm__("r1") = text;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    unsigned args[2] = {0x20026u, code};
    r0 = 0x20;
    r1 = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
#endif
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void cmx_fault(void)
{
    cmx_port_finish(25);
}
