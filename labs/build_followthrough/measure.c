#define _POSIX_C_SOURCE 200809L
#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static uint64_t now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))exit(2);return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
static volatile uint64_t sink;
int main(int argc,char **argv){
 size_t n=argc>1?(size_t)strtoul(argv[1],NULL,10):65536;unsigned rounds=argc>2?(unsigned)strtoul(argv[2],NULL,10):100;unsigned bias=argc>3?(unsigned)strtoul(argv[3],NULL,10):0;
 if(n==0||n>1048576||rounds==0||rounds>100000)return 2;
 uint32_t *a=malloc(n*sizeof *a),*b=malloc(n*sizeof *b),*o=malloc(n*sizeof *o);if(!a||!b||!o)return 2;
 uint32_t seed=3417;for(size_t i=0;i<n;++i){seed=seed*1664525u+1013904223u;a[i]=seed;b[i]=seed^0x55aa55aau;}
 for(unsigned warm=0;warm<10;++warm)bf_add(a,b,o,n);
 for(unsigned pair=0;pair<9;++pair)for(unsigned step=0;step<2;++step){unsigned variant=(pair+step)%2;uint64_t checksum=0;
  seed=3417;for(size_t i=0;i<n;++i){seed=seed*1664525u+1013904223u;a[i]=seed;b[i]=seed^0x55aa55aau;}
  uint64_t start=now();
  for(unsigned r=0;r<rounds;++r){a[r%n]+=(uint32_t)(r+pair+1);if(variant)bf_add_disjoint(a,b,o,n);else bf_add(a,b,o,n);checksum+=o[r%n];}
  uint64_t elapsed=now()-start;for(size_t i=0;i<n;++i)checksum+=o[i];sink=checksum;
  printf("sample,%zu,%u,%u,%llu,%llu\n",n,pair,variant,(unsigned long long)elapsed,(unsigned long long)checksum);
 }
 uint64_t start=now();for(unsigned r=0;r<rounds;++r){unsigned x=(r%100)<(bias?5u:95u)?3u:8u;sink+=(uint64_t)bf_clamp_branch((int)x);}printf("clamp,%u,%llu\n",bias,(unsigned long long)(now()-start));
 printf("clock,CLOCK_MONOTONIC,cache,warm,affinity,uncontrolled,seed,3417\n");free(a);free(b);free(o);return sink==UINT64_MAX?1:0;
}
