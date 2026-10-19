#ifndef CM_ABI_DEPTH_H
#define CM_ABI_DEPTH_H
#include <stddef.h>
#include <stdint.h>
struct ad_cell { uint32_t x; uint16_t y; };
struct ad_pair { uint64_t a,b; };
struct ad_triple { uint64_t a,b,c; };
struct ad_mixed { double real; uint64_t integer; };
struct ad_observation { uint64_t result,before,entry,preserved,mask; };
/* Borrow p[0..n), n<=16; null is permitted exactly when n==0. */
uint64_t ad_sum64(const uint32_t *p,size_t n);
uint32_t ad_sum32(const uint32_t *p,size_t n);
struct ad_pair ad_pair(uint64_t a,uint64_t b);
struct ad_triple ad_triple(uint64_t a,uint64_t b,uint64_t c);
struct ad_mixed ad_mixed(double real,uint64_t integer);
uint64_t ad_nine(uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,uint64_t,double);
uint32_t ad_inline_add(uint32_t,uint32_t);
uint32_t ad_named_memory(uint32_t *);
uint32_t ad_opaque_memory(uint32_t *);
void ad_hostile(const uint32_t *,size_t,const uint64_t[6],struct ad_observation *);
uint64_t ad_call8(void);
extern uint64_t ad_entry_mod,ad_before8,ad_entry8,ad_seventh,ad_eighth;
void ad_cfi(int);
void ad_trace_leaf(void);
#endif
