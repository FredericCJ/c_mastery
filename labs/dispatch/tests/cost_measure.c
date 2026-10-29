#define _POSIX_C_SOURCE 200809L
#include "dispatch_resolution.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
extern int dc_add(int,int,int *),dc_max(int,int,int *),dc_direct(const int *,int *),dc_measure(const void *,int *),dc_max_fold(const int *,size_t,int,int *);
extern ds_combine_fn dc_choose(unsigned);
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DISPATCH_COST:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static uint64_t now(void){struct timespec t;CHECK(clock_gettime(CLOCK_MONOTONIC,&t)==0);return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
static unsigned element,selector;
static int selected(int a,int b,int *out){return dc_choose(selector+element++)(a,b,out);}
static int direct_selected(const int *p,size_t n,int *out){int value=0;for(size_t i=0;i<n;++i){int next,status=(selector+i)&1?dc_max(value,p[i],&next):dc_add(value,p[i],&next);if(status)return status;value=next;}*out=value;return DS_OK;}
int main(int argc,char **argv){CHECK(argc==2);selector=(unsigned)strtoul(argv[1],NULL,10);CHECK(selector<2);int values[1000];struct ds_box boxes[1000];struct ds_view views[1000];static const struct ds_ops operations={dc_measure};for(unsigned i=0;i<1000;++i){values[i]=(int)i+1;boxes[i]=(struct ds_box){&operations,values[i]};views[i]=(struct ds_view){&values[i],&operations};}
 printf("L %zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(int),sizeof(long),sizeof(void *),sizeof(struct ds_view),_Alignof(struct ds_view),sizeof(struct ds_box),_Alignof(struct ds_box),offsetof(struct ds_box,value));
 if(!selector)for(unsigned trial=0;trial<7;++trial)for(unsigned k=0;k<3;++k){unsigned kind=(k+trial)%3;long long checksum=0;int out=0;atomic_signal_fence(memory_order_seq_cst);uint64_t start=now();for(unsigned repeat=0;repeat<100;++repeat)for(unsigned i=0;i<1000;++i){int status=kind==0?dc_direct(&values[i],&out):kind==1?ds_embedded(&boxes[i],&out):ds_measure(views[i],&out);CHECK(status==DS_OK);checksum+=out;}atomic_signal_fence(memory_order_seq_cst);uint64_t elapsed=now()-start;CHECK(checksum==50050000);printf("R %u %u %llu %lld\n",trial,kind,(unsigned long long)elapsed,checksum);}
 int inputs[16];for(unsigned i=0;i<16;++i)inputs[i]=(int)(i%7)-3;
 for(unsigned trial=0;trial<7;++trial)for(unsigned k=0;k<5;++k){unsigned kind=(k+trial)%5;long long checksum=0;int out=91;atomic_signal_fence(memory_order_seq_cst);uint64_t start=now();for(unsigned i=0;i<50000;++i){int status;if(kind==0)status=selector?dc_max_fold(inputs,16,0,&out):ds_sum(inputs,16,0,&out);else if(kind==1)status=ds_fold(inputs,16,0,dc_add,&out);else if(kind==2)status=ds_fold(inputs,16,0,dc_choose(selector),&out);else if(kind==3)status=direct_selected(inputs,16,&out);else{element=0;status=ds_fold(inputs,16,0,selected,&out);CHECK(element==16);}CHECK(status==DS_OK);checksum+=out;}atomic_signal_fence(memory_order_seq_cst);uint64_t elapsed=now()-start;printf("D %u %u %u %llu %lld\n",selector,trial,kind,(unsigned long long)elapsed,checksum);}
 if(!selector)for(unsigned population=1;population<=4;population*=2)for(unsigned trial=0;trial<7;++trial)for(unsigned k=0;k<2;++k){unsigned kind=(k+trial)%2;struct ds_resolution methods;struct ds_resolution_cache cache;ds_resolution_init(&methods);ds_resolution_cache_init(&cache);CHECK(ds_resolution_set(&methods,0,0,dc_add,NULL)==DS_OK);long long checksum=0;int out=91;atomic_signal_fence(memory_order_seq_cst);uint64_t start=now();for(unsigned i=0;i<20000;++i){unsigned type=i%population;int status=kind?ds_resolution_call(&methods,&cache,type,type,10,2,&out):ds_resolution_uncached(&methods,type,type,10,2,&out);CHECK(status==DS_OK);checksum+=out;}atomic_signal_fence(memory_order_seq_cst);uint64_t elapsed=now()-start;CHECK(checksum==240000);if(kind)CHECK(cache.misses==(population==4?20000:population)&&cache.hits+cache.misses==20000);printf("C %u %u %u %llu %lld %u %u\n",population,trial,kind,(unsigned long long)elapsed,checksum,cache.hits,cache.misses);ds_resolution_cache_clear(&cache);ds_resolution_clear(&methods);}
 return 0;}
