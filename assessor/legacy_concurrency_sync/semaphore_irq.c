#include "semaphore_irq.h"
#ifndef CDSEM_IRQ_MUTANT
#define CDSEM_IRQ_MUTANT 0
#endif
#define REG32(address) (*(volatile uint32_t *)(uintptr_t)(address))
#define REG8(address) (*(volatile uint8_t *)(uintptr_t)(address))

uint32_t cds_irq_save(void)
{
    uint32_t saved;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(saved)::"memory");
    return saved;
}

void cds_irq_restore(uint32_t saved)
{
    if (CDSEM_IRQ_MUTANT == 1)
        saved = 0;
    __asm__ volatile("msr primask,%0\ndsb\nisb" ::"r"(saved) : "memory");
}

static unsigned exception(void)
{
    uint32_t number;
    __asm__ volatile("mrs %0,ipsr" : "=r"(number));
    return number;
}

static void pend(void)
{
    REG32(0xe000ed04u) = UINT32_C(1) << 28;
    __asm__ volatile("dsb\nisb" ::: "memory");
}

int cds_irq_post(struct cd_sem *s, struct cd_post *out)
{
    unsigned number = exception();
    if (number != 0 && number != 14)
    {
        if (number < 16 || number >= 256 ||
            REG8(0xe000e400u + number - 16) < CDS_KERNEL_THRESHOLD)
            return CMX_INVALID;
    }
    uint32_t saved = cds_irq_save();
    int result = cds_post(s, out);
    if (result == CMX_OK && out->defer_schedule)
        pend();
    cds_irq_restore(saved);
    return result;
}

int cds_high_offer(struct cd_deferred *d)
{
    /* No CMX object access or scheduler call occurs in this urgent handler. */
    uint32_t saved = cds_irq_save();
    int result = cds_deferred_offer(d);
    if (CDSEM_IRQ_MUTANT == 3 && result == CMX_OK)
        --d->pending;
    if (result == CMX_OK)
        pend();
    cds_irq_restore(saved);
    return result;
}
