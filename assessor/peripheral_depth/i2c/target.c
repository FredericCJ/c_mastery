#include "i2c.h"
#ifndef PI_TARGET_MUTANT
#define PI_TARGET_MUTANT 0
#endif
#ifdef PI_TARGET_MODEL
volatile uint32_t *pi_test_register(uint32_t);
extern uint32_t pi_test_primask;
#define REG(a) (*pi_test_register(a))
#else
_Static_assert(sizeof(uintptr_t) == 4, "F439 pointer width");
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#endif
static uint32_t irq_save(void) {
#ifdef PI_TARGET_MODEL
  uint32_t key = pi_test_primask;
  pi_test_primask = 1;
  return key;
#else
  uint32_t key;
  __asm__ volatile("mrs %0, primask\n cpsid i" : "=r"(key)::"memory");
  return key;
#endif
}
static void irq_restore(uint32_t key) {
#ifdef PI_TARGET_MODEL
  pi_test_primask = key;
#else
  __asm__ volatile("msr primask, %0" ::"r"(key) : "memory");
#endif
}
static uint32_t rd(void *p, unsigned r) {
  (void)p;
  switch (r) {
  case PI_CR1:
    return REG(0x40005400u);
  case PI_CR2:
    return REG(0x40005404u);
  case PI_OAR1:
    return REG(0x40005408u);
  case PI_OAR2:
    return REG(0x4000540cu);
  case PI_DR:
    return REG(0x40005410u);
  case PI_SR1:
    return REG(0x40005414u);
  case PI_SR2:
    return REG(0x40005418u);
  case PI_CCR:
    return REG(0x4000541cu);
  case PI_TRISE:
    return REG(0x40005420u);
  case PI_FLTR:
    return REG(0x40005424u);
  default:
    return 0;
  }
}
static void wr(void *p, unsigned r, uint32_t v) {
  (void)p;
  switch (r) {
  case PI_CR1:
    REG(0x40005400u) = v;
    break;
  case PI_CR2:
    REG(0x40005404u) = v;
    break;
  case PI_OAR1:
    REG(0x40005408u) = v;
    break;
  case PI_OAR2:
    REG(0x4000540cu) = v;
    break;
  case PI_DR:
    REG(0x40005410u) = v;
    break;
  case PI_SR1:
    REG(0x40005414u) = v;
    break;
  case PI_CCR:
    REG(0x4000541cu) = v;
    break;
  case PI_TRISE:
    REG(0x40005420u) = v;
    break;
  case PI_FLTR:
    REG(0x40005424u) = v;
    break;
  default:
    break;
  }
}
PiIo pi_f439_io(void) {
  PiIo io = {0, rd, wr};
  return io;
}
static void af(void) {
  REG(0x40020424u) =
      (REG(0x40020424u) & ~255u) | (PI_TARGET_MUTANT == 1 ? 0x55u : 0x44u);
  REG(0x40020400u) = (REG(0x40020400u) & ~0x000f0000u) | 0x000a0000u;
}
void pi_f439_pins(void) {
  /* Exclusive initial setup, before any transaction. External pullups required.
   */
  REG(0x40023830u) |= 2u;
  (void)REG(0x40023830u);
  REG(0x40023840u) |= 1u << 21;
  (void)REG(0x40023840u);
  REG(0x40023820u) |= 1u << 21;
  (void)REG(0x40023820u);
  REG(0x40023820u) &= ~(1u << 21);
  REG(0x40020418u) = 0x300u;
  REG(0x40020404u) =
      (REG(0x40020404u) & ~0x300u) | (PI_TARGET_MUTANT == 2 ? 0u : 0x300u);
  REG(0x40020408u) = (REG(0x40020408u) & ~0xf0000u) | 0x50000u;
  REG(0x4002040cu) &= ~0xf0000u;
  af();
}
int pi_f439_step(PiI2c *s, uint32_t token, uint32_t now) {
  uint32_t key = irq_save();
  int result = pi_step(s, pi_f439_io(), token, now);
  irq_restore(key);
  return result;
}
int pi_f439_open(PiI2c *s, const PuClock *clock) {
  uint32_t key = irq_save();
  int result = pi_open(s, pi_f439_io(), clock);
  if (result == PI_OK)
    af();
  irq_restore(key);
  return result;
}
int pi_f439_begin(PiI2c *s, uint32_t generation, uint32_t token,
                  unsigned address, unsigned command, unsigned count,
                  uint8_t *out, uint32_t now, unsigned budget) {
  uint32_t key = irq_save();
  int result = pi_begin(s, pi_f439_io(), generation, token, address, command,
                        count, out, now, budget);
  irq_restore(key);
  return result;
}
int pi_f439_cancel(PiI2c *s, uint32_t token) {
  uint32_t key = irq_save();
  int result = pi_cancel(s, pi_f439_io(), token);
  irq_restore(key);
  return result;
}
static unsigned line_read(void *p, unsigned line) {
  (void)p;
  return line <= PI_SDA ? (REG(0x40020410u) >> (8u + line)) & 1u : 0;
}
static void line_release(void *p, unsigned line, unsigned high) {
  (void)p;
  if (line <= PI_SDA && high <= 1)
    REG(0x40020418u) = 1u << (8u + line + (high ? 0u : 16u));
}
PiLines pi_f439_lines(void) {
  PiLines io = {0, line_read, line_release};
  return io;
}
int pi_f439_recovery_prepare(PiI2c *s, unsigned exclusive, unsigned peers) {
  if (!s || exclusive > 1 || peers > 1)
    return PI_INVALID;
  if (!exclusive || peers)
    return PI_FORBIDDEN;
  if (s->phase && s->phase != 10)
    return PI_BUSY;
  uint32_t key = irq_save();
  REG(0x40023820u) |= 1u << 21;
  (void)REG(0x40023820u);
  REG(0x40023820u) &= ~(1u << 21);
  REG(0x40020418u) = 0x300u;
  REG(0x40020404u) |= 0x300u;
  REG(0x40020400u) = (REG(0x40020400u) & ~0xf0000u) | 0x50000u;
  s->phase = s->master = s->ready = 0;
  irq_restore(key);
  return PI_OK;
}
