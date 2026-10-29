#ifndef AB_BOUND_H
#define AB_BOUND_H
#include <stddef.h>
#include <stdint.h>
#include "algo.h"
_Static_assert(sizeof(algo_pair)==8,"selected assembly record stride is eight bytes");
struct ab_step { uint32_t lo,hi,mid,next_lo,next_hi; };
struct ab_trace { uint32_t index,count;struct ab_step steps[12]; };
struct ab_context { uint32_t calls,misaligned,poison; };
typedef uint32_t (*ab_read_fn)(void *,const uint32_t *,uint32_t);
int ab_trace(const uint32_t *,uint32_t,uint32_t,unsigned,struct ab_trace *);
int ab_checked(const uint32_t *,uint32_t,uint32_t,uint32_t,unsigned,uint32_t *,uint32_t *);
size_t ab_records_lower(const algo_pair *,size_t,uint32_t);
int ab_records_checked(const algo_pair *,size_t,size_t,uint32_t,size_t *);
uint32_t ab_callback_lower(const uint32_t *,uint32_t,uint32_t,ab_read_fn,void *);
uint32_t ab_poison(void *,const uint32_t *,uint32_t);
uint32_t ab_callback_entry(const uint32_t *,uint32_t,uint32_t);
int ab_callback_abi_probe(const uint32_t *,uint32_t,uint32_t,uint32_t *);
#if defined(__x86_64__)
uint32_t __attribute__((ms_abi)) ab_ms_lower(const uint32_t *,uint32_t,uint32_t);
#endif
#endif
