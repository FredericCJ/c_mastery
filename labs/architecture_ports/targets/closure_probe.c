#include <stdatomic.h>
#include <stdint.h>
#ifndef ATOMIC_WIDTH
#define ATOMIC_WIDTH 32
#endif
#if ATOMIC_WIDTH == 32
typedef uint32_t po_word;
#elif ATOMIC_WIDTH == 64
typedef uint64_t po_word;
#elif ATOMIC_WIDTH == 128
/* Explicit compiler extension, not an ISO C integer type. */
typedef unsigned __int128 po_word;
#else
#error unsupported width
#endif
static _Atomic(po_word) value;
__attribute__((noinline)) po_word closure_step(po_word delta){return atomic_fetch_add_explicit(&value,delta,memory_order_acq_rel);}
__attribute__((noinline)) uint64_t closure_divide(uint64_t a,uint64_t b){return a/b;}
/* Valid only with nonzero divisor. This image is linked for closure inspection;
   reset/privilege/stack/runtime execution is provided by the separate guests. */
void _start(void){volatile po_word old=closure_step(3);volatile uint64_t q=closure_divide((uint64_t)old+17,3);(void)q;for(;;){}}
