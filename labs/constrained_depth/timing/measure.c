#define _POSIX_C_SOURCE 200809L
#include "timing.h"
#include <errno.h>
#include <stdio.h>
#include <time.h>
static int now(clockid_t id,uint64_t *out){struct timespec value;if(clock_gettime(id,&value)||value.tv_sec<0||value.tv_nsec<0||value.tv_nsec>=1000000000||((uint64_t)value.tv_sec>UINT64_MAX/1000000000))return 0;uint64_t whole=(uint64_t)value.tv_sec*1000000000;if(UINT64_MAX-whole<(uint64_t)value.tv_nsec)return 0;*out=whole+(uint64_t)value.tv_nsec;return 1;}
int main(void){struct timespec wall_res,cpu_res;if(clock_getres(CLOCK_MONOTONIC,&wall_res)||clock_getres(CLOCK_THREAD_CPUTIME_ID,&cpu_res))return 2;printf("CT_RESOLUTION:wall=%lld,%ld cpu=%lld,%ld units=seconds,nanoseconds\n",(long long)wall_res.tv_sec,wall_res.tv_nsec,(long long)cpu_res.tv_sec,cpu_res.tv_nsec);
 for(unsigned pattern=0;pattern<2;pattern++){uint64_t wall[24];for(unsigned i=0;i<24;i++){unsigned pause=pattern?(i>=18):(i%4==0);uint64_t a,b,c,d;if(!now(CLOCK_MONOTONIC,&a)||!now(CLOCK_THREAD_CPUTIME_ID,&b))return 2;volatile uint32_t work=i+1;for(unsigned n=0;n<10000;n++)work=work*1664525u+1013904223u;if(pause){struct timespec request={0,1000000},remaining;while(nanosleep(&request,&remaining)){if(errno!=EINTR)return 2;request=remaining;}}if(!now(CLOCK_THREAD_CPUTIME_ID,&c)||!now(CLOCK_MONOTONIC,&d)||c<b||d<a)return 2;wall[i]=d-a;printf("M,%u,%u,%u,%llu,%llu,%u\n",pattern,i,pause,(unsigned long long)(d-a),(unsigned long long)(c-b),(unsigned)work);}struct ct_stats stats;if(ct_stats(wall,24,1000000,&stats)!=CT_OK)return 2;printf("S,%u,%llu,%llu,%llu,%llu,%llu,%zu,%zu\n",pattern,(unsigned long long)stats.minimum,(unsigned long long)stats.maximum,(unsigned long long)stats.sum,(unsigned long long)stats.mean_integer,(unsigned long long)stats.mean_remainder,stats.longest_exceedance_run,stats.exceedance_runs);}
 puts("CT_MEASURE_PASS:actual-host-observations-no-WCET-or-DMA-claim");return 0;
}
