#include "raw.h"
__attribute__((noreturn)) void _start(void) {
  static const char message[] = "OF:aarch64:write64:x0-x2:x8:negative-error\n";
  long ok = of_raw_write(1, message, sizeof message - 1),
       bad = of_raw_write(-1, message, 1);
  register long x0 __asm__("x0") =
      (ok == (long)(sizeof message - 1) && bad == -9) ? 0 : 1;
  register long x8 __asm__("x8") = 93;
  __asm__ volatile("svc #0" ::"r"(x0), "r"(x8) : "memory", "cc");
  for (;;) {
  }
}
