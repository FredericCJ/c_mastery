#include "semaphore_irq.h"
#ifndef CDSEM_IRQ_MUTANT
#define CDSEM_IRQ_MUTANT 0
#endif
#define REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define REG8(address) (*(volatile uint8_t *)(uintptr_t)(address))
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static struct cd_sem semaphore;
static struct cd_deferred deferred;
static volatile unsigned mode, interrupts, pend_calls, overflows, irq_number,
    pend_number;

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void stop(unsigned code)
{
    uint32_t args[2] = {0x20026u, code};
    semihost(4, code == 0   ? "CDSEM_M4_PASS\n"
                : code == 1 ? "CDSEM_M4_CHECK\n"
                            : "CDSEM_M4_FAULT\n");
    semihost(0x20, args);
    for (;;)
    {
    }
}

#define CHECK(x)     \
    do               \
    {                \
        if (!(x))    \
            stop(1); \
    } while (0)

static unsigned ipsr(void)
{
    unsigned n;
    __asm__ volatile("mrs %0,ipsr" : "=r"(n));
    return n;
}

static unsigned primask(void)
{
    unsigned n;
    __asm__ volatile("mrs %0,primask" : "=r"(n));
    return n;
}

static void basepri(unsigned value)
{
    __asm__ volatile("msr basepri,%0\ndsb\nisb" ::"r"(value) : "memory");
}

static void irq0(void)
{
    irq_number = ipsr();
    CHECK(irq_number == 16);
    struct cd_post out = {99, 98, 97, 96};
    if (mode == 0)
    {
        CHECK(cds_irq_post(&semaphore, &out) == CMX_OK && out.woke &&
              out.defer_schedule);
        CHECK(kernel.current == 0);
    }
    else
    {
        CHECK(cds_irq_post(&semaphore, &out) == CMX_INVALID &&
              out.scanned == 99);
        int result = cds_high_offer(&deferred);
        CHECK(result == CMX_OK || result == CMX_FULL);
        if (result == CMX_FULL)
        {
            ++overflows;
        }
        /* This faulty adapter control calls the scheduler in the forbidden urgent
         * handler. It is intentionally separate from the correct port source. */
        if (CDSEM_IRQ_MUTANT == 2)
        {
            (void)cmx_schedule(&kernel);
        }
        CHECK(kernel.current == 0 && semaphore.wait.nodes[1].linked &&
              semaphore.wait.nodes[2].linked);
    }
    ++interrupts;
}

static void pendsv(void)
{
    pend_number = ipsr();
    CHECK(pend_number == 14);
    uint32_t saved = cds_irq_save();
    if (mode)
    {
        unsigned done = 99;
        CHECK(cds_deferred_drain(&deferred, &semaphore, 1, &done) == CMX_OK &&
              done == 1);
        CHECK(!semaphore.wait.nodes[1].linked &&
              semaphore.wait.nodes[2].linked);
        CHECK(cds_deferred_drain(&deferred, &semaphore, 1, &done) == CMX_OK &&
              done == 1);
    }
    CHECK(cmx_schedule(&kernel) == 1);
    ++pend_calls;
    cds_irq_restore(saved);
}

static void fire(unsigned expected)
{
    REG32(0xe000e200u) = 1;
    __asm__ volatile("dsb\nisb" ::: "memory");
    for (unsigned i = 0; i < 100000 && interrupts != expected; ++i)
    {
        __asm__ volatile("nop" ::: "memory");
    }
    CHECK(interrupts == expected);
}

static void exercise(void)
{
    cmx_init(&kernel, CMX_FIXED);
    CHECK(cmx_task_add(&kernel, 9, 0, 0x20006000, 256) == 0);
    CHECK(cmx_task_add(&kernel, 3, 0, 0x20006200, 256) == 1);
    CHECK(cmx_task_add(&kernel, 5, 0, 0x20006400, 256) == 2);
    CHECK(cds_init(&semaphore, &kernel, 0, 2, 1) == CMX_OK &&
          cds_deferred_init(&deferred, 2) == CMX_OK);
    struct cw_token one, two;
    CHECK(cds_take(&semaphore, 1, cmx_forever(), &one) == CMX_BLOCKED);
    CHECK(cds_take(&semaphore, 2, cmx_forever(), &two) == CMX_BLOCKED);
    CHECK(cmx_schedule(&kernel) == 0);
    REG8(0xe000ed22u) = 0xff;
    REG8(0xe000e400u) = CDS_KERNEL_THRESHOLD;
    REG32(0xe000e100u) = 1;
    uint32_t saved = cds_irq_save();
    CHECK(saved == 0 && primask() == 1);
    uint32_t nested = cds_irq_save();
    CHECK(nested == 1);
    cds_irq_restore(nested);
    CHECK(primask() == 1);
    cds_irq_restore(saved);
    CHECK(primask() == 0);
    fire(1);
    CHECK(pend_calls == 1 && kernel.current == 1 &&
          semaphore.wait.objects[0].acquired == 1);
    CHECK(cds_take(&semaphore, 1, cmx_forever(), &one) == CMX_BLOCKED);
    CHECK(cmx_schedule(&kernel) == 0);
    CHECK(cmx_task_add(&kernel, 8, 0, 0x20006600, 256) == 3);
    mode = 1;
    REG8(0xe000e400u) = 0;
    basepri(CDS_KERNEL_THRESHOLD);
    fire(2);
    fire(3);
    fire(4);
    CHECK(pend_calls == 1 && deferred.pending == 2 && deferred.accepted == 2 &&
          overflows == 1);
    basepri(0);
    for (unsigned i = 0; i < 100000 && pend_calls != 2; ++i)
    {
        __asm__ volatile("nop" ::: "memory");
    }
    CHECK(pend_calls == 2 && pend_number == 14 && deferred.pending == 0 &&
          deferred.drained == 2);
    CHECK(semaphore.wait.objects[0].posts == 3 &&
          semaphore.wait.objects[0].acquired == 3 &&
          semaphore.wait.objects[0].semaphore.count == 0);
    CHECK(cmx_invariant(&kernel));
}

void cl_reset(void)
{
    uintptr_t source = (uintptr_t)&cl_data_load;
    for (uintptr_t p = (uintptr_t)&cl_data_start; p < (uintptr_t)&cl_data_end;
         p += 4, source += 4)
    {
        *(uint32_t *)p = *(const uint32_t *)source;
    }
    for (uintptr_t p = (uintptr_t)&cl_bss_start; p < (uintptr_t)&cl_bss_end;
         p += 4)
    {
        *(uint32_t *)p = 0;
    }
    exercise();
    stop(0);
}

static void fault(void)
{
    stop(2);
}

__attribute__((section(".vectors"), used)) const void *cdsem_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault,  fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     pendsv, fault, irq0};
