#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#ifdef VS_PPC64
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ || _CALL_ELF != 2
#error Requires actual big endian PowerPC64 ELFv2
#endif
extern long sa_raw_write(const char *, size_t);
static void emit(const char *s) {
  size_t n = 0;
  while (s[n])
    n++;
  (void)sa_raw_write(s, n);
}
#else
static void emit(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}
#endif
int puts(const char *s) {
  emit(s);
  emit("\n");
  return 0;
}
/* Original bounded formatter: only literal text, %u and %s are supported.
 * It exists for test diagnostics, and does not claim hosted printf semantics.
 */
int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  for (; *fmt; fmt++) {
    if (*fmt != '%') {
      char s[2] = {*fmt, 0};
      emit(s);
      continue;
    }
    ++fmt;
    if (*fmt == 's')
      emit(va_arg(ap, const char *));
    else if (*fmt == 'u') {
      unsigned n = va_arg(ap, unsigned);
      char b[12];
      unsigned k = 11;
      b[k] = 0;
      do {
        b[--k] = (char)('0' + n % 10);
        n /= 10;
      } while (n);
      emit(b + k);
    } else {
      emit("VS_FORMATTER_UNSUPPORTED");
      va_end(ap);
      return -1;
    }
  }
  va_end(ap);
  return 0;
}
int memcmp(const void *a, const void *b, size_t n) {
  const unsigned char *x = a, *y = b;
  for (size_t i = 0; i < n; i++) {
    if (x[i] != y[i])
      return (int)x[i] - (int)y[i];
  }
  return 0;
}
#ifndef VS_PPC64
__attribute__((noreturn)) static void stop(unsigned rc) {
  unsigned args[2] = {0x20026, rc};
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = args;
  __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
  for (;;) {
  }
}
int vs_public_tests(void);
void xf_fault(void) {
  puts("VS_TARGET_FAULT");
  stop(2);
}
void xf_target_main(void) {
  if (vs_public_tests())
    stop(1);
  puts("VS_TARGET_PASS");
  stop(0);
}
#endif
