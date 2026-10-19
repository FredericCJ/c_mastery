#define _POSIX_C_SOURCE 200809L
#include "wakeup.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
enum { ROUNDS=500 };
typedef struct {pthread_mutex_t mutex;pthread_cond_t condition;unsigned level,rings,acks;} Level;
static Level line={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,0};
static SwWorld world;
static SqHandle ids[4];
static atomic_uint complete,failed;
static SlStats producer_stats,consumer_stats;
static unsigned delivered,sum;
static int ring(void *unused,unsigned cpu){(void)unused;if(cpu!=1)abort();if(pthread_mutex_lock(&line.mutex))abort();line.level=1;line.rings++;if(pthread_cond_signal(&line.condition)||pthread_mutex_unlock(&line.mutex))abort();return 1;}
static void ack(void *unused,unsigned cpu){(void)unused;if(cpu!=1)abort();if(pthread_mutex_lock(&line.mutex))abort();line.level=0;line.acks++;if(pthread_mutex_unlock(&line.mutex))abort();}
static void wait_level(void){if(pthread_mutex_lock(&line.mutex))abort();while(!line.level)if(pthread_cond_wait(&line.condition,&line.mutex))abort();if(pthread_mutex_unlock(&line.mutex))abort();}
static void *producer(void *unused){
 (void)unused;SqEvent event;
 for(unsigned r=0;r<ROUNDS;r++){
  while(atomic_load_explicit(&complete,memory_order_acquire)!=r&&!atomic_load(&failed))sched_yield();
  if(atomic_load(&failed))return NULL;
  for(unsigned slot=0;slot<4;slot++)if(sw_wake(&world,0,1,ids[slot],&event,&producer_stats)!=SQ_OK){atomic_store(&failed,1);return NULL;}
 }
 return NULL;
}
static void *consumer(void *unused){
 (void)unused;SqEvent event;SqHandle id;
 for(unsigned r=0;r<ROUNDS;r++){
  unsigned seen=0;
  while(seen!=15){
   int result=sw_dispatch(&world,1,&id,&event,&consumer_stats);
   if(result==SQ_IDLE){int idle=sw_prepare_idle(&world,1,1,&consumer_stats);if(idle==SQ_IDLE)wait_level();else if(idle!=SW_BUSY){atomic_store(&failed,2);return NULL;}continue;}
   if(result!=SQ_OK||seen&(1u<<id.slot)){atomic_store(&failed,3);return NULL;}
   seen|=1u<<id.slot;delivered++;sum+=id.slot+1;
   if(sw_finish(&world,1,id,SQ_BLOCKED,&event,&consumer_stats)!=SQ_OK){atomic_store(&failed,4);return NULL;}
  }
  atomic_store_explicit(&complete,r+1,memory_order_release);
 }
 return NULL;
}
int main(void){
 SqEvent e;SqHandle id;
 if(sw_init(&world,2,4,(SwPort){ring,ack,NULL})!=SQ_OK)return 1;
 for(unsigned i=0;i<4;i++)if(sw_create(&world,0,1,i,&ids[i],&e)!=SQ_OK)return 2;
 for(unsigned i=0;i<4;i++)if(sw_dispatch(&world,1,&id,&e,&consumer_stats)!=SQ_OK||sw_finish(&world,1,id,SQ_BLOCKED,&e,&consumer_stats)!=SQ_OK)return 3;
 if(sw_prepare_idle(&world,1,1,&consumer_stats)!=SQ_IDLE)return 4;
 pthread_t a,b;if(pthread_create(&a,NULL,producer,NULL)||pthread_create(&b,NULL,consumer,NULL))return 5;
 if(pthread_join(a,NULL)||pthread_join(b,NULL))return 6;
 if(atomic_load(&failed)||atomic_load(&complete)!=ROUNDS||delivered!=2000||sum!=5000||!sq_invariant(&world.queues)||sw_prepare_idle(&world,1,1,&consumer_stats)!=SQ_IDLE||line.level)return 7;
 printf("SW_NATIVE_PASS:level-predicate rounds=%u delivered=%u sum=%u rings=%u acknowledgements=%u\n",ROUNDS,delivered,sum,line.rings,line.acks);return 0;
}
