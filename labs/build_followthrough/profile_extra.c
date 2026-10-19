#define _POSIX_C_SOURCE 200809L
#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>
static volatile uint64_t sink;
static uint64_t now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))exit(2);return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
int main(void){
 enum{N=65536};static int data[N];static uint32_t next[N];
 struct rusage usage_before,usage_after;if(getrusage(RUSAGE_SELF,&usage_before))return 2;
 for(unsigned pattern=0;pattern<3;++pattern){uint32_t seed=3434;uint64_t expected=0;
  for(size_t i=0;i<N;++i){seed=1664525u*seed+1013904223u;data[i]=pattern==0?3:pattern==1?(i%2?-2:5):(int)(seed%15)-5;expected+=(uint64_t)(data[i]<0?0:data[i]>4?4:data[i]);}
  for(unsigned pair=0;pair<9;++pair)for(unsigned step=0;step<2;++step){unsigned path=(pair+step)%2;uint64_t sum=0,start=now();for(size_t i=0;i<N;++i)sum+=(uint64_t)(path?bf_clamp_select(data[i]):bf_clamp_branch(data[i]));uint64_t duration=now()-start;sink=sum;
   if(sum!=expected){puts("EXTRA_CHECK:clamp");return 1;}printf("clamp_sample,%u,%u,%u,%llu,%llu\n",pattern,pair,path,(unsigned long long)duration,(unsigned long long)sum);
  }
 }
 for(unsigned si=0;si<3;++si){uint32_t stride=si==0?1:si==1?17:4097;
  for(uint32_t i=0;i<N;++i)next[(i*stride)%N]=((i+1)*stride)%N;
  for(unsigned path=0;path<2;++path){uint64_t sum=0,start=now();uint32_t current=0;
   for(uint32_t i=0;i<N;++i){uint32_t index=path?current:(i*stride)%N;sum+=index;if(path)current=next[current];}
   uint64_t duration=now()-start;sink=sum;if(sum!=((uint64_t)N*(N-1))/2){puts("EXTRA_CHECK:stride");return 1;}
   printf("stride_sample,%u,%u,%llu,%llu\n",stride,path,(unsigned long long)duration,(unsigned long long)sum);
  }
 }
 uint64_t start=now(),sum=0;for(size_t i=0;i<N;++i)sum+=(uint32_t)i;sink=sum;printf("overhead,checksum_loop,%llu\n",(unsigned long long)(now()-start));
 if(getrusage(RUSAGE_SELF,&usage_after))return 2;
 printf("OS_COUNTER,ru_minflt,%ld,%ld,delta,%ld,scope,whole-process-minor-page-faults\n",usage_before.ru_minflt,usage_after.ru_minflt,usage_after.ru_minflt-usage_before.ru_minflt);
 puts("EXTRA_PASS:constant-alternating-mixed; independent-dependent-strides; checksum-overhead");return 0;
}
