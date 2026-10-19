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
void xf_fault(void) { puts("EH_TARGET_FAULT"); finish(2); }
int eh_tests(unsigned);
int eh_stack(void);
int eh_reassembly(void);
void xf_target_main(void) {
  for (unsigned i = 0; i < 4; ++i) { int e = eh_tests(i); if (e) finish((unsigned)e); }
  int e = eh_stack(); if (e) finish((unsigned)e);
  e = eh_reassembly(); if (e) finish((unsigned)e);
  puts("EH_GUEST_PASS actual_original_stack_CPU_models_no_physical_Ethernet");
  finish(0);
}
