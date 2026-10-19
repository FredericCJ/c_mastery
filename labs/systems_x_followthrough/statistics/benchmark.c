#define _POSIX_C_SOURCE 200809L
#include "kernel.h"
#include "systems.h"
#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include <limits.h>
static volatile uint64_t consumed;
static uint32_t data[4096];
static uint32_t seed=UINT32_C(0x71ace2);
static uint32_t random_word(void){seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);return seed;}
static int sample(unsigned which,uint32_t *out){
 struct timespec a,b;uint64_t result=0,ns;
 if(clock_gettime(CLOCK_MONOTONIC,&a))return 1;
 for(unsigned i=0;i<64;i++)result+=which?sx_sum_b(data,4096):sx_sum_a(data,4096);
 consumed^=result;
 if(clock_gettime(CLOCK_MONOTONIC,&b)||result!=UINT64_C(536739840))return 1;
 ns=(uint64_t)(b.tv_sec-a.tv_sec)*UINT64_C(1000000000)+(uint64_t)b.tv_nsec-(uint64_t)a.tv_nsec;
 if(ns>UINT32_MAX)return 1;
 *out=(uint32_t)ns;return 0;
}
int main(void){uint32_t a[32],b[32],warm;unsigned order[32],wins;int64_t median;struct timespec resolution;
 for(unsigned i=0;i<4096;i++)data[i]=i;
 for(unsigned i=0;i<16;i++){if(sample(0,&warm)||sample(1,&warm))return 2;}
 for(unsigned i=0;i<32;i++)order[i]=i%2;
 for(unsigned i=31;i>0;i--){unsigned j=random_word()%(i+1),v=order[i];order[i]=order[j];order[j]=v;}
 if(clock_getres(CLOCK_MONOTONIC,&resolution))return 2;
 printf("META seed=7449826 warmup=16 pairs=32 repeats=64 clock=MONOTONIC resolution=%ld:%ld\n",resolution.tv_sec,resolution.tv_nsec);
 for(unsigned i=0;i<32;i++){
  if(order[i]){if(sample(1,&b[i])||sample(0,&a[i]))return 2;}
  else{if(sample(0,&a[i])||sample(1,&b[i]))return 2;}
  printf("PAIR %u %s %" PRIu32 " %" PRIu32 "\n",i,order[i]?"BA":"AB",a[i],b[i]);
 }
 if(s_paired(a,b,32,&median,&wins)!=S_OK)return 1;
 printf("PAIRED median=%" PRId64 " wins=%u consumed=%" PRIu64 "\n",median,wins,consumed);return 0;
}
