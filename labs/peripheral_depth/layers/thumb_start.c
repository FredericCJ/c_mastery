#include <stdint.h>
/* Startup primitives derived from labs/algorithm_sort/thumb_start.c.
 * Separate model guest: no physical STM32 peripheral is emulated. */
int pl_guest_main(void);
extern uint32_t __stack_top, __data_start, __data_end, __data_load, __bss_start,
    __bss_end;
static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) void pl_stop(unsigned code) {
  uint32_t args[2] = {0x20026u, code};
  semihost(4, code ? "PL_GUEST_FAIL\n" : "PL_GUEST_PASS\n");
  semihost(0x20, args);
  for (;;) {
  }
}
void pl_report(const char *s) {
  semihost(4, s);
  semihost(4, "\n");
}
void reset(void) {
  uint32_t *src = &__data_load;
  for (uint32_t *p = &__data_start; p < &__data_end;)
    *p++ = *src++;
  for (uint32_t *p = &__bss_start; p < &__bss_end;)
    *p++ = 0;
  if (pl_guest_main())
    pl_stop(1);
  pl_stop(0);
}
static void fault(void) { pl_stop(2); }
__attribute__((section(".vectors"), used)) const uintptr_t vectors[] = {
    (uintptr_t)&__stack_top, (uintptr_t)reset, (uintptr_t)fault,
    (uintptr_t)fault,        (uintptr_t)fault, (uintptr_t)fault,
    (uintptr_t)fault};
