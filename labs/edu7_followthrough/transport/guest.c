static void print(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
int puts(const char *s) { print(s); print("\n"); return 0; }
__attribute__((noreturn)) static void finish(unsigned code) {
  unsigned data[2] = {0x20026, code};
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = data;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
  for (;;) {}
}
void xf_fault(void) { puts("ET_TARGET_FAULT"); finish(2); }
int et_tests(unsigned);
void xf_target_main(void){for(unsigned i=0;i<3;i++){int r=et_tests(i);if(r)finish((unsigned)r);}puts("ET_GUEST_PASS actual_Thumb_original_EDU7_no_physical_RS485");finish(0);}
