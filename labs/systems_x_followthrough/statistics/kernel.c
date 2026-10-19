#include "kernel.h"
/* Equivalent actual consumed workloads, compiled separately without LTO. */
uint64_t sx_sum_a(const uint32_t*p,size_t n){uint64_t sum=0;for(size_t i=0;i<n;i++)sum+=p[i];return sum;}
uint64_t sx_sum_b(const uint32_t*p,size_t n){uint64_t x=0,y=0;size_t i=0;for(;n-i>=2;i+=2){x+=p[i];y+=p[i+1];}if(i<n)x+=p[i];return x+y;}
