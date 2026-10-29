#include "notifications_irq.h"
#ifndef CDN_IRQ_MUTANT
#define CDN_IRQ_MUTANT 0
#endif
#define REG32(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define REG8(a) (*(volatile uint8_t *)(uintptr_t)(a))

uint32_t cdn_irq_save(void)
{
    uint32_t value;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(value)::"memory");
    return value;
}

void cdn_irq_restore(uint32_t value)
{
    if (CDN_IRQ_MUTANT == 1)
        value = 0;
    __asm__ volatile("msr primask,%0\ndsb\nisb" ::"r"(value) : "memory");
}

static unsigned exception(void)
{
    unsigned value;
    __asm__ volatile("mrs %0,ipsr" : "=r"(value));
    return value;
}

int cdn_irq_post(struct cdn_handle h, uint32_t count, struct cdn_post *out)
{
    unsigned number = exception();
    if (CDN_IRQ_MUTANT != 3 && number != 0 && number != 14)
        if (number < 16 || number >= 256 ||
            REG8(0xe000e400u + number - 16) < CDN_KERNEL_THRESHOLD)
            return CMX_INVALID;
    uint32_t saved = cdn_irq_save();
    int result = cdn_post(h, count, out);
    if (result == CMX_OK && out->defer_schedule)
    {
        REG32(0xe000ed04u) = UINT32_C(1) << 28;
        __asm__ volatile("dsb\nisb" ::: "memory");
        if (CDN_IRQ_MUTANT == 2)
            (void)cmx_schedule(h.owner->kernel);
    }
    cdn_irq_restore(saved);
    return result;
}

int cdn_thread_wait(struct cdn_handle h, uint32_t up_to, cmx_deadline deadline)
{
    uint32_t primask, basepri, faultmask;
    __asm__ volatile("mrs %0,primask\nmrs %1,basepri\nmrs %2,faultmask"
                     : "=r"(primask), "=r"(basepri), "=r"(faultmask));
    if (exception() || primask || basepri || faultmask)
        return CMX_INVALID;
    uint32_t saved = cdn_irq_save();
    int result = cdn_wait(h, up_to, deadline);
    cdn_irq_restore(saved);
    return result;
}
