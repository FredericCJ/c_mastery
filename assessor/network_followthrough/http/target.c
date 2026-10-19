#include "http.h"
static void print(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
int puts(const char *s) {
  print(s);
  print("\n");
  return 0;
}
__attribute__((noreturn)) static void finish(unsigned code) {
  unsigned data[2] = {0x20026, code};
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = data;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
  for (;;) {
  }
}
void xf_fault(void) {
  puts("NH_TARGET_FAULT");
  finish(2);
}
int nh_public_tests(void);
int nb_board_tests(void);

void xf_target_main(void) {
  if (nh_public_tests() || nb_board_tests())
    finish(1);
  puts("NH_TARGET_PASS actual_ring_ARP_IP_TCP_HTTP "
       "CPU_state_only");
  finish(0);
}
