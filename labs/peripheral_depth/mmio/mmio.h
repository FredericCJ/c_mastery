#ifndef PM_MMIO_H
#define PM_MMIO_H
#include "firmware_io.h"
#include <stddef.h>
#include <stdint.h>

enum
{
    PM_CTRL = 0,
    PM_STATUS = 4,
    PM_DATA = 8,
    PM_RTC = 12,
    PM_ODR = 16,
    PM_BSRR = 20
};

struct pm_access
{
    unsigned offset, width, write;
    uint32_t value;
};

struct pm_model
{
    uint32_t control, status, rtc, odr;
    unsigned revision, rx_count, tx_count, log_count;
    unsigned char rx[4], last_tx;
    struct pm_access log[64];
};

/* Caller initializes revision1/2, optional synthetic status/receive events. */
int pm_access(struct pm_model *, unsigned offset, unsigned width,
              unsigned write, uint32_t *value);
int pm_gpio(struct fw_io, unsigned set, unsigned clear);

struct pm_request
{
    unsigned phase, remaining;
    unsigned char byte;
};

int pm_begin(struct pm_request *, unsigned char byte, unsigned polls);
/*0 complete;1 waiting;-1 invalid;-2 exhausted. Same step for poll/IRQ delivery.
 */
int pm_step(struct pm_request *, struct pm_model *);

struct pm_interrupt
{
    struct pm_request request;
    uint32_t start, limit, last;
    unsigned armed;
};

int pm_irq_begin(struct pm_interrupt *, struct pm_model *, unsigned char,
                 uint32_t now, uint32_t ticks);
/* Serialized IRQ/deadline dispatcher. Readiness at deadline wins. */
int pm_irq_event(struct pm_interrupt *, struct pm_model *, uint32_t now);

struct pm_filter
{
    unsigned stable, candidate, count, threshold, events, initialized;
    uint32_t last, since, duration, max_gap;
};

int pm_filter_init(struct pm_filter *, unsigned threshold, uint32_t duration,
                   uint32_t max_gap);
int pm_sample(struct pm_filter *, unsigned level, uint32_t now,
              unsigned *changed);
/*0 low,1 high,2 floating,3 contention. Abstract two-driver network. */
int pm_line(unsigned open_drain, unsigned output, unsigned peer_low,
            unsigned pull_up);

struct pm_gpio
{
    volatile uint32_t moder, otyper, ospeedr, pupdr;
    volatile const uint32_t idr;
    volatile uint32_t odr, bsrr, lckr, afr[2];
};

_Static_assert(offsetof(struct pm_gpio, idr) == 0x10 &&
                   offsetof(struct pm_gpio, bsrr) == 0x18 &&
                   offsetof(struct pm_gpio, afr) == 0x20 &&
                   sizeof(struct pm_gpio) == 0x28,
               "GPIO layout");
uint32_t pm_target_read(const struct pm_gpio *);
int pm_target_write(struct pm_gpio *, unsigned set, unsigned clear);
#endif
