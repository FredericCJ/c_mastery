#ifndef OS_FOLLOW_RAW_H
#define OS_FOLLOW_RAW_H
#include <stddef.h>
/* Original hosted Linux wrappers. Selected ABI, not portable ISO C. */
static inline long of_raw_write(int fd, const void *data, size_t n) {
#if defined(__x86_64__)
  long result = 1;
  __asm__ volatile("syscall"
                   : "+a"(result)
                   : "D"((long)fd), "S"(data), "d"(n)
                   : "rcx", "r11", "memory", "cc");
  return result;
#elif defined(__aarch64__)
  register long x0 __asm__("x0") = fd;
  register const void *x1 __asm__("x1") = data;
  register size_t x2 __asm__("x2") = n;
  register long x8 __asm__("x8") = 64;
  __asm__ volatile("svc #0"
                   : "+r"(x0)
                   : "r"(x1), "r"(x2), "r"(x8)
                   : "memory", "cc");
  return x0;
#else
#error Selected Linux ABI not implemented
#endif
}
#endif
