#include <stdint.h>
#include <stdatomic.h>
uint32_t divide32(uint32_t n,uint32_t d){return n/d;}
uint64_t divide64(uint64_t n,uint64_t d){return n/d;}
typedef struct {uint32_t words[37];} Copy;
void copy_value(Copy *d,const Copy *s){*d=*s;}
uint32_t atomic32(_Atomic uint32_t *p){return atomic_fetch_add_explicit(p,1,memory_order_acq_rel);}
uint64_t atomic64(_Atomic uint64_t *p){return atomic_fetch_add_explicit(p,1,memory_order_acq_rel);}
/* This inspection object is intentionally NOT linked into the target image.
 * A64-bit atomic fallback is a dependency question, not permission to link
 * libatomic or to replace atomicity with ordinary loads/stores. */
