#include "adc.h"
#ifndef PA_TARGET_MUTANT
#define PA_TARGET_MUTANT 0
#endif
#ifdef PA_TARGET_MODEL
volatile uint32_t *pa_test_register(uint32_t);
#define REG(a) (*pa_test_register(a))
#else
_Static_assert(sizeof(uintptr_t) == 4, "F439 pointer width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#endif
static uint32_t rd(void *p, unsigned r) {
  (void)p;
  switch (r) {
  case PA_SR:
    return REG(0x40012000u);
  case PA_CR1:
    return REG(0x40012004u);
  case PA_CR2:
    return REG(0x40012008u);
  case PA_SMPR1:
    return REG(0x4001200cu);
  case PA_SMPR2:
    return REG(0x40012010u);
  case PA_SQR1:
    return REG(0x4001202cu);
  case PA_SQR2:
    return REG(0x40012030u);
  case PA_SQR3:
    return REG(0x40012034u);
  case PA_DR:
    return REG(PA_TARGET_MUTANT == 1 ? 0x4001214cu : 0x4001204cu);
  case PA_CCR:
    return REG(0x40012304u);
  default:
    return 0;
  }
}
static void wr(void *p, unsigned r, uint32_t v) {
  (void)p;
  switch (r) {
  case PA_SR:
    REG(0x40012000u) = v;
    break;
  case PA_CR1:
    REG(0x40012004u) = v;
    break;
  case PA_CR2:
    REG(0x40012008u) = v;
    break;
  case PA_SMPR1:
    REG(0x4001200cu) = v;
    break;
  case PA_SMPR2:
    REG(0x40012010u) = v;
    break;
  case PA_SQR1:
    REG(0x4001202cu) = v;
    break;
  case PA_SQR2:
    REG(0x40012030u) = v;
    break;
  case PA_SQR3:
    REG(0x40012034u) = v;
    break;
  case PA_CCR:
    REG(0x40012304u) = v;
    break;
  default:
    break;
  }
}
struct pa_io pa_f439_io(void) {
  struct pa_io io = {0, rd, wr};
  return io;
}
void pa_f439_pins(void) {
  REG(0x40023830u) |= 1u;
  (void)REG(0x40023830u);
  REG(0x40020000u) |= 15u;
  REG(0x4002000cu) &= ~15u;
  REG(0x40023844u) |= 1u << 8;
  (void)REG(0x40023844u);
}
int pa_f439_timer(unsigned prescaler, unsigned reload, unsigned enabled) {
  if (prescaler > 65535 || reload < 1 || reload > 65535 || enabled > 1)
    return PA_INVALID;
  if (enabled) {
    if (REG(0x40000428u) != prescaler || REG(0x4000042cu) != reload)
      return PA_INVALID;
    REG(0x40000400u) |= 1;
    return PA_OK;
  }
  REG(0x40023840u) |= 2;
  (void)REG(0x40023840u);
  if (PA_TARGET_MUTANT != 4) {
    /* Exclusive timer reset removes inherited slave/external-clock/output
     * state. */
    REG(0x40023820u) |= 2;
    (void)REG(0x40023820u);
    REG(0x40023820u) &= ~2u;
    (void)REG(0x40023820u);
  }
  REG(0x40000400u) = 0;
  REG(0x40000404u) = 0;
  REG(0x40000428u) = prescaler;
  REG(0x4000042cu) = reload;
  REG(0x40000414u) = 1;
  REG(0x40000410u) = 0;
  REG(0x40000404u) = PA_TARGET_MUTANT == 2 ? 0x10u : 0x20u;
  return PA_OK;
}
void pa_f439_dac_open(void) {
  REG(0x40023830u) |= 1;
  (void)REG(0x40023830u);
  REG(0x40020000u) |= 3u << 8;
  REG(0x4002000cu) &= ~(3u << 8);
  REG(0x40023840u) |= 1u << 29;
  (void)REG(0x40023840u);
  REG(0x40007400u) = 0x3du;
}
int pa_f439_dac_write(unsigned code) {
  if (code > 4095)
    return PA_INVALID;
  REG(0x40007408u) = PA_TARGET_MUTANT == 3 ? code << 4 : code;
  REG(0x40007404u) = 1;
  return PA_OK;
}
unsigned pa_f439_dac_read(void) { return REG(0x4000742cu) & 4095u; }
