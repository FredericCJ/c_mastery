#include "cl_cmx_irq.h"
#ifndef CC_IRQ_MUTANT
#define CC_IRQ_MUTANT 0
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))

int cc_irq_receive(struct cc_handle h, size_t n, cmx_deadline deadline,
                   struct cc_ticket *out)
{
    uint32_t previous = cw_irq_save();
    cc_irq_enrollment_hook();
    int result = cc_receive(h, n, deadline, out);
    cw_irq_restore(previous);
    return result;
}

int cc_irq_publish(struct cc_port p, uint32_t producer, uint32_t sequence,
                   const unsigned char *bytes, size_t length)
{
    uint32_t previous = cw_irq_save();
    int result = cc_port_send(p, producer, sequence, bytes, length);
    cw_irq_restore(previous);
    return result;
}

int cc_irq_join0(struct cc_port *p)
{
    uint32_t ipsr;
    __asm__ volatile("mrs %0,ipsr" : "=r"(ipsr)::"memory");
    if (ipsr || !p)
    {
        return CC_INVALID;
    }
    uint32_t previous = cw_irq_save();
    cc_irq_join_hook();
    if (CC_IRQ_MUTANT != 1)
    {
        REG(0xe000e180) = 1;
    }
    __asm__ volatile("dsb\nisb" ::: "memory");
    if (CC_IRQ_MUTANT != 2)
    {
        REG(0xe000e280) = 1;
    }
    __asm__ volatile("dsb\nisb" ::: "memory");
    int result =
        (REG(0xe000e100) & 1) || (REG(0xe000e200) & 1) || (REG(0xe000e300) & 1)
            ? CL_BUSY
            : cc_port_release_after_quiescence(p);
    cw_irq_restore(previous);
    return result;
}
