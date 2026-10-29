#include "adc.h"
#ifndef PA_COMP_MUTANT
#define PA_COMP_MUTANT 0
#endif
#ifdef PA_TARGET_MODEL
volatile uint32_t *pa_test_register(uint32_t);
#define REG(a) (*pa_test_register(a))
#else
_Static_assert(sizeof(uintptr_t) == 4, "G474 pointer width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#endif
int pa_g474_comp_open(struct pa_comparator *s, unsigned hysteresis) {
  if (!s || hysteresis > 7)
    return PA_INVALID;
  if (s->phase)
    return PA_BUSY;
  REG(0x40021060u) |= 1;
  (void)REG(0x40021060u);
  if (REG(0x40010200u) & UINT32_C(0x80000001))
    return PA_BUSY;
  REG(0x4002104cu) |= 1;
  (void)REG(0x4002104cu);
  REG(0x48000000u) |= 15;
  REG(0x4800000cu) &= ~15u;
  /* Official CMSIS addresses and inspected vendor LL selection constants.
     No HAL/LL code is linked. External PA1 positive and PA0 negative inputs. */
  REG(0x40010200u) =
      (hysteresis << 16) | (PA_COMP_MUTANT == 1 ? 0x60u : 0x70u) | 1;
  s->phase = 1;
  s->warm_sampled = 0;
  return PA_OK;
}
int pa_g474_comp_poll(struct pa_comparator *s, uint32_t now, unsigned *level) {
  if (!s || !level)
    return PA_INVALID;
  if (!s->phase)
    return PA_STALE;
  if (!s->warm_sampled) {
    s->warm_sampled = 1;
    s->last = now;
    s->due = now + 6;
    return PA_BUSY;
  }
  if (now - s->last >= UINT32_C(0x80000000))
    return PA_INVALID;
  s->last = now;
  if (PA_COMP_MUTANT != 2 && now - s->due >= UINT32_C(0x80000000))
    return PA_BUSY;
  *level = (REG(0x40010200u) >> 30) & 1;
  s->phase = 2;
  return PA_OK;
}
int pa_g474_comp_close(struct pa_comparator *s) {
  if (!s)
    return PA_INVALID;
  if (REG(0x40010200u) & UINT32_C(0x80000000))
    return PA_BUSY;
  REG(0x40010200u) = 0;
  s->phase = 0;
  return PA_OK;
}
