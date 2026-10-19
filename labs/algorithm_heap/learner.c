#include "heap.h"
int ah_trace(uint32_t*a,uint32_t n,uint32_t capacity,uint32_t p,unsigned arity,unsigned minimum,AHStats*s,AHTrace*t){(void)a;(void)n;(void)capacity;(void)p;(void)arity;(void)minimum;(void)s;(void)t;return 7;}
void ah_heap_init(CDHeap*h,unsigned m){(void)h;(void)m;}
enum CDStatus ah_heap_insert(CDHeap*h,int p,unsigned i,CDHeapHandle*out){(void)h;(void)p;(void)i;(void)out;return CD_FAILED;}
enum CDStatus ah_heap_take(CDHeap*h,int*p,unsigned*i){(void)h;(void)p;(void)i;return CD_FAILED;}
enum CDStatus ah_heap_get(const CDHeap*h,CDHeapHandle k,int*p,unsigned*i){(void)h;(void)k;(void)p;(void)i;return CD_FAILED;}
enum CDStatus ah_heap_change(CDHeap*h,CDHeapHandle k,int p){(void)h;(void)k;(void)p;return CD_FAILED;}
enum CDStatus ah_heap_build(CDHeap*h,const int*a,size_t n,unsigned m){(void)h;(void)a;(void)n;(void)m;return CD_FAILED;}
/* Prior artifact is separately required; these unsolved import stubs are intentional. */
void cd_heap_init(CDHeap*h,unsigned m){ah_heap_init(h,m);}
enum CDStatus cd_heap_insert(CDHeap*h,int p,unsigned i,CDHeapHandle*out){return ah_heap_insert(h,p,i,out);}
enum CDStatus cd_heap_take(CDHeap*h,int*p,unsigned*i){return ah_heap_take(h,p,i);}
enum CDStatus cd_heap_change(CDHeap*h,CDHeapHandle k,int p){return ah_heap_change(h,k,p);}
enum CDStatus cd_heap_build(CDHeap*h,const int*a,size_t n,unsigned m){return ah_heap_build(h,a,n,m);}
