#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static unsigned comparisons;
static int cmp(const void*a,const void*b){++comparisons;int x=*(const int*)a,y=*(const int*)b;return(x>y)-(x<y);}
static int record_cmp(const void*a,const void*b){++comparisons;const struct ef_record*x=a,*y=b;volatile unsigned checksum=0;for(unsigned i=0;i<32;++i)checksum+=x->payload[i]+y->payload[i];(void)checksum;return(x->key>y->key)-(x->key<y->key);}
static uint64_t now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))abort();return(uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
int main(void){
 unsigned checksum=0;printf("{\"samples\":[");
 for(unsigned sample=0;sample<25;++sample){uint64_t start=now();for(unsigned repeat=0;repeat<2000;++repeat){int a[16];for(unsigned i=0;i<16;++i)a[i]=(int)((i*11+repeat)%7);if(ef_sort(a,16)!=EF_OK)abort();checksum+=(unsigned)a[0];}uint64_t bounded=now()-start;
 comparisons=0;start=now();for(unsigned repeat=0;repeat<2000;++repeat){int a[16];for(unsigned i=0;i<16;++i)a[i]=(int)((i*11+repeat)%7);qsort(a,16,sizeof*a,cmp);checksum+=(unsigned)a[0];}uint64_t library=now()-start;
 printf("%s{\"bounded_ns\":%llu,\"qsort_ns\":%llu,\"qsort_comparisons\":%u}",sample?",":"",(unsigned long long)bounded,(unsigned long long)library,comparisons);}
 struct ef_record records[16];for(unsigned i=0;i<16;++i){records[i].key=(int)(15-i);records[i].identity=i;memset(records[i].payload,(int)i,32);}comparisons=0;uint64_t start=now();qsort(records,16,sizeof*records,record_cmp);uint64_t large=now()-start;
 for(unsigned i=0;i<16;++i)if(records[i].key!=(int)i)abort();
 printf("],\"large_record_ns\":%llu,\"large_record_comparisons\":%u,\"checksum\":%u,\"contract\":\"ascending keys and permutation; qsort stability not assumed\"}\n",(unsigned long long)large,comparisons,checksum);return 0;
}
