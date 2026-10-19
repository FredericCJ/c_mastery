#include "sleep.h"
volatile uint32_t mf_sleep_irqs,mf_sleep_early;
void mf_clock_init(void){mf_trap();}
uint32_t mf_clock_now(void){return 0;}
void mf_nap(MfNap *s,uint32_t n,unsigned b){(void)s;(void)n;(void)b;mf_trap();}
int mf_nap_finish(MfNap *s){(void)s;return 0;}
