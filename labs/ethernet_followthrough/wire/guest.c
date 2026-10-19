#include "wire.h"
int ew_tests(unsigned);
static void semi(uint32_t op, const void *p) {
  register uint32_t a __asm__("r0") = op;
  register const void *b __asm__("r1") = p;
  __asm__ volatile("bkpt 0xab" : "+r"(a) : "r"(b) : "memory");
}
static void text(const char *s) { semi(4, s); }
void ew_note(const char *s) {
  text(s);
  text("\n");
}
static void number(uint32_t n) {
  char s[12];
  unsigned i = 11;
  s[i] = 0;
  do {
    s[--i] = (char)('0' + n % 10);
    n /= 10;
  } while (n);
  text(s + i);
}
void ew_observation(const char *s, const uint32_t *v, size_t n) {
  text("OBS wire_");
  text(s);
  for (size_t i = 0; i < n; ++i) {
    text(" ");
    number(v[i]);
  }
  text("\n");
}
static void stop(unsigned n) {
  uint32_t p[] = {0x20026u, n};
  semi(0x20, p);
  for (;;) {
  }
}
void xf_fault(void) {
  text("WIRE_GUEST_FAULT\n");
  stop(42);
}
void xf_capture(uint32_t *s, uint32_t r, unsigned i, uint32_t m) {
  (void)s;
  (void)r;
  (void)i;
  (void)m;
  xf_fault();
}
void xf_target_main(void) {
  for (unsigned i = 0; i < 6; ++i)
    if (ew_tests(i))
      stop(41);
  text("WIRE_GUEST_PASS actual-Thumb synthetic-bytes no-physical-Ethernet\n");
  stop(0);
}
