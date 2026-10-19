#ifndef PM_MMIO_H
#define PM_MMIO_H
#include "firmware_io.h"
#include <stddef.h>
#include <stdint.h>
enum {
  PM_CTRL = 0,
  PM_STATUS = 4,
  PM_DATA = 8,
  PM_RTC = 12,
  PM_ODR = 16,
  PM_BSRR = 20
};
typedef struct {
  unsigned offset, width, write;
  uint32_t value;
} PmAccess;
typedef struct {
  uint32_t control, status, rtc, odr;
  unsigned revision, rx_count, tx_count, log_count;
  unsigned char rx[4], last_tx;
  PmAccess log[64];
} PmModel;
/* Caller initializes revision1/2, optional synthetic status/receive events. */
int pm_access(PmModel *, unsigned offset, unsigned width, unsigned write,
              uint32_t *value);
int pm_gpio(FwIo, unsigned set, unsigned clear);
typedef struct {
  unsigned phase, remaining;
  unsigned char byte;
} PmRequest;
int pm_begin(PmRequest *, unsigned char byte, unsigned polls);
/*0 complete;1 waiting;-1 invalid;-2 exhausted. Same step for poll/IRQ delivery.
 */
int pm_step(PmRequest *, PmModel *);
typedef struct {
  PmRequest request;
  uint32_t start, limit, last;
  unsigned armed;
} PmInterrupt;
int pm_irq_begin(PmInterrupt *, PmModel *, unsigned char, uint32_t now,
                 uint32_t ticks);
/* Serialized IRQ/deadline dispatcher. Readiness at deadline wins. */
int pm_irq_event(PmInterrupt *, PmModel *, uint32_t now);
typedef struct {
  unsigned stable, candidate, count, threshold, events, initialized;
  uint32_t last, since, duration, max_gap;
} PmFilter;
int pm_filter_init(PmFilter *, unsigned threshold, uint32_t duration,
                   uint32_t max_gap);
int pm_sample(PmFilter *, unsigned level, uint32_t now, unsigned *changed);
/*0 low,1 high,2 floating,3 contention. Abstract two-driver network. */
int pm_line(unsigned open_drain, unsigned output, unsigned peer_low,
            unsigned pull_up);
typedef struct {
  volatile uint32_t moder, otyper, ospeedr, pupdr;
  volatile const uint32_t idr;
  volatile uint32_t odr, bsrr, lckr, afr[2];
} PmGpio;
_Static_assert(offsetof(PmGpio, idr) == 0x10 &&
                   offsetof(PmGpio, bsrr) == 0x18 &&
                   offsetof(PmGpio, afr) == 0x20 && sizeof(PmGpio) == 0x28,
               "GPIO layout");
uint32_t pm_target_read(const PmGpio *);
int pm_target_write(PmGpio *, unsigned set, unsigned clear);
#endif
