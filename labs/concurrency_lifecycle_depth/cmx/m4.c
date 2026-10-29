#include <stdint.h>
#include "cl_cmx_irq.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;

static void cc_semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void cc_target_stop(unsigned code)
{
    uint32_t args[2] = {0x20026u, code};
    cc_semihost(4, code == 0   ? "CC_M4_PASS\n"
                   : code == 1 ? "CC_M4_CHECK\n"
                               : "CC_M4_FAULT\n");
    cc_semihost(0x20, args);
    for (;;)
    {
    }
}

static int cc_target_puts(const char *s)
{
    cc_semihost(4, s);
    cc_semihost(4, "\n");
    return 0;
}

#define CC_TARGET 1
#include "tests.c"
#define MMIO(a) (*(volatile uint32_t *)(uintptr_t)(a))
static cmx_kernel irq_kernel;
static struct cc_queue irq_queue;
static struct cc_handle irq_handles[5];
static struct cc_port irq_port;
static volatile unsigned irq_entries, enroll_probe, join_probe;

void cw_irq_enrollment_hook(void)
{
}

void cc_irq_enrollment_hook(void)
{
    if (enroll_probe)
    {
        uint32_t mask;
        __asm__ volatile("mrs %0,primask" : "=r"(mask));
        CHECK(mask == 1);
        MMIO(0xe000e200) = 1;
        __asm__ volatile("dsb\nisb" ::: "memory");
        CHECK(irq_entries == 0);
    }
}

void cc_irq_join_hook(void)
{
    if (join_probe)
    {
        uint32_t mask;
        __asm__ volatile("mrs %0,primask" : "=r"(mask));
        CHECK(mask == 1);
        MMIO(0xe000e200) = 1;
        __asm__ volatile("dsb\nisb" ::: "memory");
        CHECK(MMIO(0xe000e200) & 1);
    }
}

static void cc_irq0(void)
{
    uint32_t ipsr;
    __asm__ volatile("mrs %0,ipsr" : "=r"(ipsr));
    CHECK(ipsr == 16 && (MMIO(0xe000e300) & 1));
    irq_entries++;
    CHECK(cc_irq_join0(&irq_port) == CC_INVALID);
    unsigned char payload[4] = {7, 3, 0, 9};
    CHECK(cc_irq_publish(irq_port, 8, irq_entries, payload, 4) == CL_OK);
}

static void cc_irq_experiment(void)
{
    CHECK(cq_setup(&irq_queue, &irq_kernel, irq_handles) &&
          cc_port_acquire(&irq_queue, &irq_port) == CL_OK);
    MMIO(0xe000e280) = 1;
    MMIO(0xe000e100) = 1;
    struct cc_ticket ticket;
    struct cl_message out;
    enroll_probe = 1;
    CHECK(cc_irq_receive(irq_handles[4], 4, cmx_forever(), &ticket) ==
          CC_BLOCKED);
    enroll_probe = 0;
    for (unsigned i = 0; i < 1000 && !irq_entries; i++)
    {
        __asm__ volatile("nop");
    }
    CHECK(irq_entries == 1);
    uint32_t outer = cw_irq_save();
    CHECK(outer == 0);
    CHECK(irq_kernel.task[4].state == CMX_READY &&
          cc_result(ticket, &out) == CL_OK && out.producer == 8 &&
          out.bytes[0] == 7 && out.bytes[3] == 9);
    /* The second enrollment preserves an already-masked caller. */
    CHECK(cc_irq_receive(irq_handles[4], 4, cmx_forever(), &ticket) ==
          CC_BLOCKED);
    uint32_t mask;
    __asm__ volatile("mrs %0,primask" : "=r"(mask));
    CHECK(mask == 1);
    MMIO(0xe000e200) = 1;
    __asm__ volatile("dsb\nisb" ::: "memory");
    CHECK(irq_entries == 1);
    cw_irq_restore(outer);
    for (unsigned i = 0; i < 1000 && irq_entries < 2; i++)
    {
        __asm__ volatile("nop");
    }
    CHECK(irq_entries == 2);
    outer = cw_irq_save();
    CHECK(outer == 0);
    CHECK(cc_result(ticket, &out) == CL_OK && out.sequence == 2);
    CHECK(cc_close(&irq_queue, 0) == CL_OK);
    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(cc_release(&irq_handles[i]) == CL_OK);
    }
    CHECK(cc_destroy(&irq_queue) == CL_BUSY);
    struct cc_port stale = irq_port;
    join_probe = 1;
    CHECK(cc_irq_join0(&irq_port) == CL_OK);
    join_probe = 0;
    __asm__ volatile("mrs %0,primask" : "=r"(mask));
    CHECK(mask == 1);
    CHECK(!(MMIO(0xe000e100) & 1) && !(MMIO(0xe000e200) & 1) &&
          !(MMIO(0xe000e300) & 1));
    CHECK(cc_destroy(&irq_queue) == CL_OK && cc_restart(&irq_queue) == CL_OK);
    CHECK(cc_port_send(stale, 8, 3, 0, 0) == CL_STALE);
    cw_irq_restore(outer);
    for (unsigned i = 0; i < 1000; i++)
    {
        __asm__ volatile("nop");
    }
    CHECK(irq_entries == 2);
    cc_semihost(
        4,
        "CC_M4_IRQ:actual-IRQ0x2;copied-payload-wakes-CMX-task;outer-mask-preserved;handler-join-refused;disabled+pending-cleared+inactive-before-lease-drop\n");
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
    CHECK(cq_queue() == 0 && cq_terminal() == 0 && cq_condition() == 0 &&
          cq_lifetime() == 0 && cq_dispatch() == 0);
    cc_irq_experiment();
    cc_target_stop(0);
}

static void fault(void)
{
    cc_target_stop(2);
}

__attribute__((section(".vectors"), used)) const void *cc_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault,  fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault, cc_irq0};
