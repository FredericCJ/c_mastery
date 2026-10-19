#include "sync.h"
#ifndef SU_TARGET_MODEL
_Static_assert(sizeof(uintptr_t)==4,"F439 address width");
#define R(a) (*(volatile uint32_t *)(uintptr_t)(a))
#else
volatile uint32_t *su_test_register(uint32_t);
#define R(a) (*su_test_register(a))
#endif
void su_f439_ck(void) {
  /* Exclusive PD10 ownership. GPIO clock readback follows retained RCC rule. */
  R(0x40023830u)|=8u; (void)R(0x40023830u);
  R(0x40020c00u)=(R(0x40020c00u)&~(3u<<20))|(2u<<20);
  R(0x40020c04u)&=~(1u<<10);
  R(0x40020c08u)&=~(3u<<20);
  R(0x40020c0cu)&=~(3u<<20);
  R(0x40020c24u)=(R(0x40020c24u)&~(15u<<8))|(7u<<8);
}
