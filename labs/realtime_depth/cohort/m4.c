#include <stdint.h>
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static void semihost(uint32_t op, const void *argument) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = argument;
  __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}
__attribute__((noreturn)) static void su_target_stop(unsigned code) {
  uint32_t args[2] = {0x20026u, code};
  semihost(4, code == 0   ? "SU_M4_PASS\n"
              : code == 1 ? "SU_M4_CHECK\n"
                          : "SU_M4_FAULT\n");
  semihost(0x20, args);
  for (;;) {
  }
}
static void su_target_puts(const char *text) {
  semihost(4, text);
  semihost(4, "\n");
}
#define SU_TARGET
#include "tests.c"
static void fault(void) { su_target_stop(2); }
__attribute__((noreturn)) void cl_reset(void) {
  uint32_t *source = &cl_data_load;
  for (uint32_t *p = &cl_data_start; p < &cl_data_end;)
    *p++ = *source++;
  for (uint32_t *p = &cl_bss_start; p < &cl_bss_end;)
    *p++ = 0;
  CHECK(!cohort() && !overrun() && !guard() && !comparison());
  su_target_stop(0);
}
__attribute__((section(".vectors"), used))
const uintptr_t vectors[16] = {(uintptr_t)&cl_stack_top,
                               (uintptr_t)cl_reset,
                               (uintptr_t)fault,
                               (uintptr_t)fault,
                               (uintptr_t)fault,
                               (uintptr_t)fault,
                               (uintptr_t)fault,
                               0,
                               0,
                               0,
                               0,
                               (uintptr_t)fault,
                               (uintptr_t)fault,
                               0,
                               (uintptr_t)fault,
                               (uintptr_t)fault};
