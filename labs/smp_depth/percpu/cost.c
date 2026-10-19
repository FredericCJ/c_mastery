#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
typedef struct {_Alignas(128) _Atomic uint64_t value;unsigned char pad[120];} Padded;
static Padded padded[2];static _Atomic uint64_t packed[2];static pthread_barrier_t start;
typedef struct {unsigned slot,pad;int cpu,error;uint64_t count;} Args;
static void *worker(void *v){Args *a=v;cpu_set_t set;CPU_ZERO(&set);CPU_SET(a->cpu,&set);a->error=pthread_setaffinity_np(pthread_self(),sizeof set,&set);pthread_barrier_wait(&start);if(a->error)return NULL;_Atomic uint64_t *p=a->pad?&padded[a->slot].value:&packed[a->slot];for(uint64_t i=0;i<a->count;++i)atomic_fetch_add_explicit(p,1,memory_order_relaxed);return NULL;}
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))abort();return (uint64_t)t.tv_sec*1000000000+(uint64_t)t.tv_nsec;}
int main(void){cpu_set_t allowed;CPU_ZERO(&allowed);if(sched_getaffinity(0,sizeof allowed,&allowed))return 2;int ids[2],n=0;for(int i=0;i<CPU_SETSIZE&&n<2;++i)if(CPU_ISSET(i,&allowed))ids[n++]=i;if(n<2){fputs("SP_COST_UNAVAILABLE:two-allowed-logical-CPUs\n",stderr);return 2;}
 for(unsigned pair=0;pair<10;++pair)for(unsigned step=0;step<2;++step){unsigned pad=(pair+step)&1;atomic_store(&padded[0].value,0);atomic_store(&padded[1].value,0);atomic_store(&packed[0],0);atomic_store(&packed[1],0);if(pthread_barrier_init(&start,NULL,3))return 3;Args a[2]={{0,pad,ids[0],0,500000},{1,pad,ids[1],0,500000}};pthread_t threads[2];for(unsigned i=0;i<2;++i)if(pthread_create(&threads[i],NULL,worker,&a[i]))return 3;
  uint64_t begin=ns();pthread_barrier_wait(&start);for(unsigned i=0;i<2;++i)if(pthread_join(threads[i],NULL)||a[i].error)return 3;uint64_t elapsed=ns()-begin;pthread_barrier_destroy(&start);uint64_t x=atomic_load(pad?&padded[0].value:&packed[0]),y=atomic_load(pad?&padded[1].value:&packed[1]);if(x!=500000||y!=500000)return 1;
  printf("SP_COST pair=%u padded=%u cpus=%d,%d counts=%llu,%llu elapsed_ns=%llu stride=%zu line=%ld\n",pair,pad,ids[0],ids[1],(unsigned long long)x,(unsigned long long)y,(unsigned long long)elapsed,pad?sizeof(Padded):sizeof(packed[0]),sysconf(_SC_LEVEL1_DCACHE_LINESIZE));
 }
 return 0;
}
