#include "mutable_views.h"
#if !defined(__x86_64__) || !defined(__SSE2__)
#error This declared backend requires the hosted x86-64 SSE2 profile
#endif
#ifndef ADV_MUTATION
#define ADV_MUTATION 0
#endif
typedef uint64_t AdvLanes __attribute__((vector_size(16)));
uint64_t adv_vector4(const uint32_t *p){
 if(ADV_MUTATION==5)return (uint32_t)(p[0]+p[1])+(uint64_t)(uint32_t)(p[2]+p[3]);
 AdvLanes a={p[0],p[1]},b={p[2],ADV_MUTATION==6?0:p[3]};
 AdvLanes sum=a+b;return sum[0]+sum[1];
}
